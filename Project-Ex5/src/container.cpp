#include <iostream>
#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <array>
#include <vector>
#include <pthread.h>

const size_t STACK_SIZE = 8192;

/** A barrier that ensures the container doesn't execve before the parent (container builder)
  * has configured Cgroups, to avoid race conditions in which the container starts
  * too many processes before the parent had restricted him. */
// TODO - consider using mmap() instead of breaking things up with CLONE_VM
pthread_barrier_t cgroup_barrier;

struct ChildArgs {
    std::string new_hostname;
    std::string new_filesystem_path;
    std::string program_path;
    std::vector<std::string> program_args;
};


[[ noreturn ]] void panic(const std::string &failedFunc) {
    auto errMsg = "system error: " + failedFunc + " - ";

    perror(errMsg.c_str());
    exit(1);
}

int fetchChildArgs(int argc, char *const *argv, ChildArgs &child_args) {
    child_args = {
            .new_hostname = argv[1],
            .new_filesystem_path = argv[2],
            .program_path = argv[4],
            .program_args = {}
    };

    for (auto program_args_idx = 5; program_args_idx < argc; ++program_args_idx) {
        child_args.program_args.emplace_back(argv[program_args_idx]);
    }

    return std::stoi(argv[3]);
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

    if (mount("proc", "/proc", "proc", 0, nullptr )) {
        panic("mount()");
    }
}

int child(void* arg) {
    ChildArgs &args = *static_cast<ChildArgs*>(arg);
    setContainerNS(args);

    // Wait on barrier till container builder (essentially the parent) configures suitable cgroups
    if (pthread_barrier_wait(&cgroup_barrier) == EINVAL) {
        perror("child cgroup_barrier: ");
    }

    // execve stuff:
    const char* c_program_path = args.program_path.c_str();
    std::vector<const char *> c_program_args = toCArgs(args, c_program_path);
    execve(c_program_path, const_cast<char *const*>(c_program_args.data()), nullptr);
    perror("system error: execve() - ");

    return 1;
}

void configureCgroups(int child_pid, int num_processes) {
    (void)child_pid;
    (void)num_processes;
    // TODO - implement this one, in order to limit the container's num of processes
}

// TODO - Yikes! Massive refactoring is required below; Consider introducing useful classes/structs.

int main(int argc, char **argv) {
    ChildArgs child_args;

    int max_n_processes = fetchChildArgs(argc, argv, child_args);

    // TODO: Consider migrating barrier stuff to a RAII class
    pthread_barrierattr_t attr;
    if (pthread_barrierattr_init(&attr)) {
        perror("pthread_barrierattr_init: ");
        exit(1);
    }

    if (pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
        perror("pthread_barrierattr_setpshared: ");
        exit(1);
    }

    if (pthread_barrier_init(&cgroup_barrier, &attr, 2)) {
        perror("pthread_barrier_init: ");
        exit(1);
    }

    std::array<uint8_t, STACK_SIZE> stack{};
    int child_pid = clone(child, stack.data() + stack.size(),
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD | CLONE_VM,
                          &child_args
    );

    if (child_pid < 0) {
        panic("clone()");
    }

    // Let the child spawn at most #max_n_processes processes
    configureCgroups(child_pid, max_n_processes);

    // Goto barrier, indicating cgroups are successfully configured
    if (pthread_barrier_wait(&cgroup_barrier) == EINVAL) {
        perror("parent cgroup_barrier: ");
        exit(1);
    }

    if (pthread_barrier_destroy(&cgroup_barrier)) {
        perror("parent pthread_barrier_destroy: ");
        exit(1);
    }

    if (pthread_barrierattr_destroy(&attr)) {
        perror("pthread_barrierattr_destroy: ");
        exit(1);
    }

    wait(nullptr); // Wait until child terminates

    std::string proc_path = child_args.new_filesystem_path + "/proc";
    if (umount2(proc_path.c_str(), MNT_DETACH)) {
        panic("umount2() proc");
    }

    return 0;
}