#include <iostream>
#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <array>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <sys/stat.h>


const size_t STACK_SIZE = 8192;

const std::string CGROUP_DIR = "/sys/fs/cgroup/shumayev-cgroup";

struct ChildArgs {
    std::string new_hostname;
    std::string new_filesystem_path;
    std::string program_path;
    std::vector<std::string> program_args;
    /** A barrier that ensures the container doesn't execve before the parent (container builder)
     * has configured Cgroups, to avoid race conditions in which the container starts
     * too many processes before the parent had restricted him. */
    pthread_barrier_t *cgroup_barrier;
};


[[ noreturn ]] void panic(const std::string &failedFunc) {
    auto errMsg = "system error: " + failedFunc + " - ";

    perror(errMsg.c_str());
    exit(1);
}

int fetchChildArgs(int argc, char *const *argv, ChildArgs &child_args) {
    child_args = {
        argv[1],
        argv[2],
        argv[4],
        {},
        static_cast<pthread_barrier_t*>(mmap(nullptr, sizeof(pthread_barrier_t),
                                                           PROT_READ | PROT_WRITE,
                                                           MAP_ANONYMOUS | MAP_SHARED, -1, 0))
    };

    for (auto program_args_idx = 5; program_args_idx < argc; ++program_args_idx) {
        child_args.program_args.emplace_back(argv[program_args_idx]);
    }

    return std::stoi(argv[3]); // Assuming the fourth argument is an integer
}

std::vector<const char *> toCArgs(ChildArgs &args, const char *c_program_path) {
    std::vector<const char*> c_program_args = { c_program_path };

    for (const std::string& prog_arg: args.program_args) {
        c_program_args.push_back(prog_arg.c_str());
    }
    c_program_args.push_back(nullptr); // Null-termination indicator

    return c_program_args;
}

void setContainerNS(const ChildArgs &args) {
    if (sethostname(args.new_hostname.c_str(), args.new_hostname.size())) {
        panic("sethostname()");
    }

    if (chroot(args.new_filesystem_path.c_str())) {
        panic("chroot()");
    }

    if (chdir("/")) {
        panic("chdir()");
    }

    if (mount("proc", "/proc", "proc", 0, nullptr)) {
        panic("mount()");
    }
}

int child(void* arg) {
    ChildArgs &args = *static_cast<ChildArgs*>(arg);
    setContainerNS(args);

    // Wait on barrier till container builder (essentially the parent) configures
    // suitable Cgroups
    if (pthread_barrier_wait(args.cgroup_barrier) == EINVAL) {
        perror("child cgroup_barrier() ");
    }
    if (munmap(args.cgroup_barrier, sizeof(pthread_barrier_t))) {
        panic("munmap() pthread_barrier_t ");
    }

    // execve stuff:
    const char* c_program_path = args.program_path.c_str();
    std::vector<const char *> c_program_args = toCArgs(args, c_program_path);
    execve(c_program_path, const_cast<char *const*>(c_program_args.data()), nullptr);

    // Reaching this point after executing execve indicates on a failure
    perror("system error: execve() - ");
    return EXIT_FAILURE;
}

void configureCgroups(int child_pid, int num_processes) {
    if (mkdir(CGROUP_DIR.c_str(), 755) && errno != EEXIST) {
        panic("mkdir() couldn't create cgroup");
    }

    std::ofstream procs_file { CGROUP_DIR + "/cgroup.procs"};
    std::ofstream pids_max { CGROUP_DIR + "/pids.max"};
    std::ofstream notify_file { CGROUP_DIR + "/notify_on_release" };

    procs_file << child_pid << std::endl;
    pids_max << num_processes << std::endl;
    notify_file << 1 << std::endl;
}

void cleanupCgroups() {
    if (rmdir(CGROUP_DIR.c_str())) {
        panic("Couldn't remove cgroup directory");
    }
}

// TODO - Yikes! Massive refactoring is required below; Consider introducing useful classes/structs.

void barrierInit(ChildArgs &child_args, pthread_barrierattr_t &attr) {
    auto n_procs_in_barrier = 2; // Parent process and the container (its child)

    if (pthread_barrierattr_init(&attr)) {
        panic("pthread_barrierattr_init()");
    }

    if (pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
        panic("pthread_barrierattr_setpshared()");
    }

    if (pthread_barrier_init(child_args.cgroup_barrier, &attr, n_procs_in_barrier)) {
        panic("pthread_barrier_init()");
    }
}

void barrierDestroy(ChildArgs &child_args, pthread_barrierattr_t &attr) {
    if (pthread_barrier_destroy(child_args.cgroup_barrier)) {
        panic("parent pthread_barrier_destroy: ");
    }

    if (pthread_barrierattr_destroy(&attr)) {
        panic("pthread_barrierattr_destroy: ");
    }

    if (munmap(child_args.cgroup_barrier, sizeof(pthread_barrier_t))) {
        panic("munmap () pthread_barrier_t");
    }
}

int main(int argc, char **argv) {
    ChildArgs child_args;
    int max_n_processes = fetchChildArgs(argc, argv, child_args);

    // TODO: Consider migrating barrier stuff to a RAII class
    pthread_barrierattr_t attr;
    barrierInit(child_args, attr);

    std::array<uint8_t, STACK_SIZE> stack{};
    int child_pid = clone(child, stack.data() + stack.size(),
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
                          &child_args
    );

    if (child_pid < 0) {
        panic("clone()");
    }

    // Let the child spawn at most #max_n_processes processes
    configureCgroups(child_pid, max_n_processes);

    // Goto barrier, indicating Cgroups are successfully configured
    if (pthread_barrier_wait(child_args.cgroup_barrier) == EINVAL) {
        panic("parent cgroup_barrier: ");
    }
    barrierDestroy(child_args, attr);

    wait(nullptr); // Wait until child terminates

    std::string proc_path = child_args.new_filesystem_path + "/proc";
    if (umount2(proc_path.c_str(), MNT_DETACH)) {
        panic("umount2() proc");
    }

    cleanupCgroups();

    return 0;
}