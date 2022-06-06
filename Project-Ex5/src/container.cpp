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
pthread_barrier_t cgroup_barrier;

struct ChildArgs {
    std::string new_hostname;
    std::string new_filesystem_path;
    std::string program_path;
    std::vector<std::string> program_args;
};


void configureCgroups(int child_pid, int num_processes) {
    (void)child_pid;
    (void)num_processes;
    // TODO
}

// TODO - Yikes! Massive refactoring is required below.

int child(void* arg) {
    ChildArgs &args = *static_cast<ChildArgs*>(arg);

    if (sethostname(args.new_hostname.c_str(), args.new_hostname.size())) {
        perror("system error: sethostname() - ");
        exit(1);
    }

    if (chroot(args.new_filesystem_path.c_str())) {
        perror("system error: chroot() - ");
        exit(1);
    }

    if (chdir("/")) {
        perror("system error: chdir() - ");
        exit(1);
    }

    if (mount("proc", "/proc", "proc", 0, nullptr )) {
        perror("system error: mount() proc - ");
        exit(1);
    }

    if (pthread_barrier_wait(&cgroup_barrier) == EINVAL) {
        perror("child cgroup_barrier: ");
    }

    const char* c_program_path = args.program_path.c_str();
    std::vector<const char*> c_program_args = { c_program_path };
    for (const std::string& prog_arg: args.program_args) {
        c_program_args.push_back(prog_arg.c_str());
    }

    c_program_args.push_back(nullptr);
    execve(c_program_path, const_cast<char *const*>(c_program_args.data()), nullptr);
    perror("system error: execve() - ");

    return 1;
}


int main(int argc, char **argv) {
    ChildArgs child_args = {
            .new_hostname = argv[1],
            .new_filesystem_path = argv[2],
            .program_path = argv[4],
            .program_args = {}
    };

    std::string num_processes_s = argv[3];
    int num_processes = std::stoi(num_processes_s);

    for (int i = 5; i < argc; ++i) {
        child_args.program_args.emplace_back(argv[i]);
    }
    // TODO validate args

    pthread_barrierattr_t attr;
    if (pthread_barrierattr_init(&attr)) {
        perror("pthread_barrierattr_init: ");
        exit(1);
    }
    if (pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
        perror("pthread_barrierattr_setpshared: ");
        exit(1);
    }
    if (pthread_barrier_init(&cgroup_barrier, &attr, 1)) {
        perror("pthread_barrier_init: ");
        exit(1);
    }

    std::array<uint8_t, STACK_SIZE> stack{};
    int child_pid = clone(child, stack.data() + stack.size(),
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
                          &child_args
    );

    if (child_pid < 0) {
        perror("system error: clone() - ");
        exit(1);
    }

    configureCgroups(child_pid, num_processes);

    if (pthread_barrier_wait(&cgroup_barrier) == EINVAL) {
        perror("parent cgroup_barrier: ");
        exit(1);
    }

    if (pthread_barrier_destroy(&cgroup_barrier)) {
        perror("parent pthread_barrier_destroy: ");
        exit(1);
    }

    wait(nullptr);

    std::string proc_path = child_args.new_filesystem_path + "/proc";
    if (umount2(proc_path.c_str(), MNT_DETACH)) {
        perror("system error: umount() proc - ");
        exit(1);
    }

    return 0;
}