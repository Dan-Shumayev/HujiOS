//
// Created by dan-os on 31/05/2021.
//

#ifndef PROJECT_EX2_SCHEDULER_H
#define PROJECT_EX2_SCHEDULER_H

#include "thread.h" // User-Thread class
#include <signal.h> // sigaction, sigset_t
#include <list> // std::list
#include <map> // std::map
#include <sys/time.h> // itimerval
#include <stdexcept> // std::domain_error
#include <unordered_map> // std::unordered_map
#include <deque> // std::deque
#include <unordered_set> // std::unordered_set

using threadSharedPtr = std::shared_ptr<Thread>;
typedef void threadEntryPoint(void);

/**
 * Round-Robin scheduler for user-level threads.
 */
class Scheduler {
    // Private members
    /** Thread control structures: */

    /** All thread (ready, running and blocked) IDs mapping to respective thread objects.
        unordered map is good for pure lookup-retrieval purposes - Search, insertion, and removal O(1) */
    std::unordered_map<int, Thread> threads_;

    /** Queue of ready thread IDs */
    std::deque<int> readyQueue_;

    /** Currently running thread's ID */
    int currentRunningThread_;

    /** Thread's ID to be terminated during the next running thread */
    int tidToTerminate_;

    /** Unordered (don't careset of blocked thread ID's */
    std::unordered_set<int> blockedThreadSet;

    std::list<threadSharedPtr> thread_ready_list_;
    std::map<size_t, threadSharedPtr> tid_to_thread_map_;
        /** Accounting information: */
    int thread_quantum_;
    int total_quantum_;
        /** Signal-handling: */
    struct itimerval virtual_timer_; // Dedicated to time the running thread's execution
    struct sigaction signal_handler_;
    sigset_t blocked_signals_;

    /**
     * @return Returns the the lowest unused thread ID. If no unused ID, return -1.
     */
    int _getLowestAvailableId() const;

    /**
     * Check if a thread whose id=tid exists
     * @param tid Thread ID to look for
     * @return True iff a thread with the given ID exists
     */
    bool _isThreadExist(int tid);

    /**
     * Preempts the running thread, resuming the next ready thread
     */
    void _preempt();

public:
    // there will be one instance created and the library calls will be forwarded to it
    Scheduler(int quantum_usecs);

    // prohibit copying Scheduler objects
    Scheduler(const Scheduler&) = delete;
    Scheduler operator=(const Scheduler&) = delete;

    /** Spawns a new thread at the end of the ready queue
     * @param function Thread entry function
     * @return ID of newly spawned thread on success, or error code.
     */
    int spawnThread(threadEntryPoint function);

    /**
     * Terminating a thread whose ID is tid. Terminating the main thread (tid=0) results in process termination.
     * In case no thread with such a tid, an error is raised.
     * @param tid Thread ID
     * @return 0 on success, otherwise -1 indicating error code, or nothing
     */
    int terminateThread(int tid);

    /**
     * @return ID of the calling (currently running) thread
     */
    inline int getTid() const {return running_thread_;}

    /**
    * @return Total number of quantums that occupied the CPU so far
    */
    inline int getTotalQuantums() const {return total_quantum_;}

    /**
     * @param tid Thread ID
     * @return Number of quantums started by the given thread, otherwise -1 in case no such
     * thread
     */
    int getThreadQuantums(int tid);

    /**
     * Blocks a thread, may cause a schedule operation
     * @param tid Thread id to be blocked, mustn't be the main thread (==0)
     * @return 0 on success, else -1 indicating error code
     */
    int blockThread(int tid);

    /**
     * Resumes a thread.
     * @param tid Thread id to be resumed. May or may not be blocked already.
     * @return 0 on success, else -1 indicating error code
     */
    int resumeThread(int tid);
};


#endif //PROJECT_EX2_SCHEDULER_H
