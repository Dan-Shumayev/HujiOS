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

const size_t STACK_SIZE = 8192;

int child(void* arg);

struct ChildArgs {
    std::string new_hostname;
    std::string new_filesystem_path;
    std::string program_path;
    std::vector<std::string> program_args;
};

int main(int argc, char **argv) {
    ChildArgs child_args = {
        .new_hostname = argv[1],
        .new_filesystem_path = argv[2],
        .program_path = argv[4],
        .program_args = {}
    };

    std::string num_processes_s = argv[3];
    int num_processes = std::stoi(num_processes_s);

    // TODO cgroups
    (void)num_processes;

    for (int i=5; i < argc; ++i)
    {
        child_args.program_args.emplace_back(argv[i]);
    }
    // TODO validate args

    std::array<uint8_t, STACK_SIZE> stack;
    int child_pid = clone(child, stack.data() + stack.size(), 
                       CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
                       &child_args
                       );

    if (child_pid < 0) {
        perror("system error: clone() -");
        exit(1);
    }
    
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
        perror("system error: mount() - ");
        exit(1);
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