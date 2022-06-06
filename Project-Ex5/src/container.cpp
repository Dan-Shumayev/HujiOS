#include <iostream>
#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/syscall.h>    /* Definition of SYS_* constants */
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <array>
#include <vector>
#include <pthread.h>

const size_t STACK_SIZE = 8192;

int child(void* arg);

struct ChildArgs {
    std::string new_hostname;
    std::string new_filesystem_path;
    std::string program_path;
    std::vector<std::string> program_args;
};


void configureCgroups(int child_pid, int num_processes) {
    (void)child_pid;
    (void)num_processes;
    std::cout << "configure cgroups" << std::endl;
}

// A barrier that ensures the container doesn't execve before the parent(container builder)
// has configured cgroups, to avoid race conditions in which the container starts too many
// processes.
pthread_barrier_t cgroup_barrier;  

int main(int argc, char **argv) {
    ChildArgs child_args = {
        .new_hostname = argv[1],
        .new_filesystem_path = argv[2],
        .program_path = argv[4],
        .program_args = {}
    };

    std::string num_processes_s = argv[3];
    int num_processes = std::stoi(num_processes_s);

    for (int i=5; i < argc; ++i)
    {
        child_args.program_args.emplace_back(argv[i]);
    }
    // TODO validate args

    // TODO validate pthread return codes
    pthread_barrierattr_t attr; 
    int res1= pthread_barrierattr_init(&attr);
    int res2= pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); 
    int res3= pthread_barrier_init(&cgroup_barrier, &attr, 1);
    printf("res1: %d\tres2: %d\tres3: %d\n", res1, res2, res3);

    std::array<uint8_t, STACK_SIZE> stack;
    int child_pid = clone(child, stack.data() + stack.size(), 
                       CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
                       &child_args
                       );

    if (child_pid < 0) {
        perror("system error: clone() -");
        exit(1);
    }
    
    configureCgroups(child_pid, num_processes);
    int waitRes = pthread_barrier_wait(&cgroup_barrier);
    printf("parent waited finished, res: %d\n", waitRes);
    if (waitRes != 0) {
        perror("parent waitRes: ");
    }
    pthread_barrier_destroy(&cgroup_barrier);

    wait(nullptr);

    std::string proc_path = child_args.new_filesystem_path + "/proc";
    int umount_res = umount2(proc_path.c_str(), MNT_DETACH);
    if (umount_res != 0) {
        perror("system error: umount() proc - ");
        exit(1);
    }
    return 0;
}


int child(void* arg) {
    ChildArgs &args = *static_cast<ChildArgs*>(arg);

    int hostname_res = sethostname(args.new_hostname.c_str(), args.new_hostname.size());
    if (hostname_res != 0) {
        perror("system error: sethostname() - ");
        exit(1);
    }

    int chroot_res = chroot(args.new_filesystem_path.c_str());
    if (chroot_res != 0) {
        perror("system error: chroot() - ");
        exit(1);
    }

    int chdir_res = chdir("/");
    if (chdir_res != 0) {
        perror("system error: chdir() - ");
        exit(1);
    }

    int mount_res = mount("proc", "/proc", "proc", 0, 0 );
    if (mount_res != 0) {
        perror("system error: mount() proc - ");
        exit(1);
    }

    printf("container waiting barrier\n");
    int childWait = pthread_barrier_wait(&cgroup_barrier);
    printf("container waited finished, res=%d\n", childWait);
    if (childWait != 0) {
        perror("child waitRes: ");
    }

    const char* c_program_path = args.program_path.c_str();
    std::vector<const char*> c_program_args = { c_program_path };
    for (const std::string& arg: args.program_args) {
        c_program_args.push_back(arg.c_str());
        
    } 
    c_program_args.push_back(nullptr);
    execve(c_program_path, const_cast<char *const*>(c_program_args.data()), nullptr);
    perror("system error: execve() - ");
    return 1;
}