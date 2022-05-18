#ifndef PROJECT_EX2_UTHREAD_UTILITIES_H
#define PROJECT_EX2_UTHREAD_UTILITIES_H

#include <signal.h>
#include <utility>
#include <cstdlib> // std::exit
#include <iostream> // std::cerr
#include <cstring> // std::strerror
#include <ostream> // std::endl

using threadEntryPoint = void(*)();
using TidToSleepTime = std::pair<int, int>;

const int EXIT_FAIL = -1;

struct sleepTimeCmp
{
    bool operator()(const TidToSleepTime &a, const TidToSleepTime &b)
    {
        return a.second < b.second;
    };
};

/**
 * Prints to cerr a system call error and exits the program
 * @param msg Error info
 */
[[ noreturn ]] void uthreadSystemException(const char* msg);

/**
 * Prints to cerr an a thread library error and returns -1
 * @param msg Error info
 * @return -1 indicating error code
 */
int uthreadException(const char* msg);

/**
 *  When an object of this class in scope, it ensures that given signal is masked
 */
class SigMask
{
    sigset_t sigset_; // Set of signals to be masked
public:
    /** Apply the signal-masking
     * @param signo ID Number of a signal to be masked
     */
    explicit SigMask(int signo);

    /** Destroys the mask, unmask (unblock) the masked signal */
    ~SigMask();

    /** Prohibit copying signal-masking */
    SigMask(SigMask&) = delete;
    SigMask operator=(SigMask&) = delete;
};


#endif //PROJECT_EX2_UTHREAD_UTILITIES_H
