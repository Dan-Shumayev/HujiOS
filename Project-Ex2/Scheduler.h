//
// Created by dan-os on 31/05/2021.
//

#ifndef PROJECT_EX2_SCHEDULER_H
#define PROJECT_EX2_SCHEDULER_H

#include "Thread.h" // User-Thread class
#include <signal.h> // sigaction, sigset_t
#include <list> // std::list
#include <map> // std::map
#include <sys/time.h> // itimerval

typedef std::shared_ptr<Thread> = threadSharedPtr;

/**
 * Round-Robin scheduler for user-level threads.
 */
class Scheduler {
    // Private members TODO - consider making some of them public (?)
        /** Thread structures: */
    threadSharedPtr running_thread_; /** Shared_ptr bc it'd be also stored inside
                                                                        data structures */
    std::list<threadSharedPtr> thread_ready_list_;
    std::map<size_t, threadSharedPtr> tid_to_thread_map_;
    // TODO - blocked threads (by another thread/mutex) structures?
        /** Accounting information: */
    size_t total_quantum_;
    size_t min_available_tid_; /** TODO - What if there's more than one available ID
                                    between running threads? Should we maintain some
                                    data structure to maintain minimum available IDs? */
        /** Signal-handling: */
    struct itimerval virtual_timer_; // Dedicated to time the running thread's execution
    struct sigaction signal_handler_;

public:
    // TODO - How to construct this class? What's the purpose?
};


#endif //PROJECT_EX2_SCHEDULER_H
