//
// Created by dan-os on 31/05/2021.
//

#include "Scheduler.h"


Scheduler::Scheduler(int quantum_usecs)
: thread_quantum_(quantum_usecs), // TODO - int to size_t - not dangerous?
    min_available_tid_(1)
    // TODO - What else should be initialized?
{}

void Scheduler::mask_signal(const int signo)
{
    if (sigaddset(&blocked_signals_, signo)) /** TODO - is it okay to pass signo to it?
                                                    sigaddset() receives non-const int */
    {
        throw std::domain_error(MASK_SIGNAL_ERR);
    }
    else if (sigprocmask(SIG_BLOCK, &blocked_signals_, nullptr))
    {
        throw std::domain_error(MASK_SIGNAL_ERR);
    }
}

void Scheduler::unmask_signals()
{
    if (sigprocmask(SIG_UNBLOCK, &blocked_signals_, nullptr))
    {
        throw std::domain_error(UNMASK_SIGNAL_ERR);
    }
}
