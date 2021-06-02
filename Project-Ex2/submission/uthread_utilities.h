//
// Created by dan-os on 01/06/2021.
//

#ifndef PROJECT_EX2_UTHREAD_UTILITIES_H
#define PROJECT_EX2_UTHREAD_UTILITIES_H

#include <signal.h>

using threadEntryPoint = void(*)(void);

const int EXIT_FAIL = -1;

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
    SigMask(int signo);

    /** Destroys the mask, unmask (unblock) the masked signal */
    ~SigMask();

    /** Prohibit copying signal-masking */
    SigMask(SigMask&) = delete;
    SigMask operator=(SigMask&) = delete;
};


#endif //PROJECT_EX2_UTHREAD_UTILITIES_H
