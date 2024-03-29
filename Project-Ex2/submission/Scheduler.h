//
// Created by dan-os on 31/05/2021.
//

#ifndef PROJECT_EX2_SCHEDULER_H
#define PROJECT_EX2_SCHEDULER_H

#include "thread.h"      // User-Thread class
#include <signal.h>      // sigaction, sigset_t
#include <sys/time.h>    // itimerval
#include <unordered_map> // std::unordered_map
#include <deque>         // std::deque
#include <unordered_set> // std::unordered_set
#include <vector>        // std::vector
#include <set>           // std::multiset

/**
 * Round-Robin scheduler for user-level threads.
 * This class is a singleton.
 */
class Scheduler
{
public:
    /** Enumerate a thread's execution state */
    enum class PreemptReason
    {
        Termination,
        QuantumExpiration,
        Blocking
    };

private:
    /** Thread control structures: */

    /** All threads' (ready, running and blocked) IDs mapping to respective thread objects.
        unordered map is good for pure lookup-retrieval purposes - Search, insertion, and removal O(1) */
    std::unordered_map<int, Thread> threads_;

    /** Queue of ready thread IDs */
    std::deque<int> readyQueue_;

    /** unordered (don't care about the order as there could be "gaps", and no need for order) */
    std::unordered_set<int> blockedThreads_;

    /** multiset holding mappings of sleeping threads - sleeping_tid -> endSleepingQuantum. This multiset utilizes
     *  a compare function, such that the threads with the lowest `endSleepingQuantum` can be captured by rbegin().
     *  It's multi-set because there may be several tids with the same `endSleepingQuantum`. */
    std::multiset<TidToSleepTime, sleepTimeCmp> sleepThreads_;

    /** Currently running thread's ID */
    int currentRunningThread_;

    /** Thread's ID to be terminated during the next running thread */
    int tidToTerminate_;

    /** Accounting information: */
    int total_quantum_;

    struct sigaction sigAlarm_;

    int threadQuantum_; // the time in micro-seconds for each thread to occupy the CPU

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
     * @param reason The reason for preemption the running thread
     */
    void _preempt(PreemptReason preemptReason);

    /**
     * Preempts the running thread, resuming the next ready thread
     * @param tid The ready thread whose id==tid to delete from the ready queue
     */
    void _deleteReadyThread(int tid);

    /**
     * Sets timer signal alarm for the entire process. Every quantum_usecs micro-seconds the
     * signal is raised, scheduling the next thread.
     * @param quantum_usecs Each thread is occupying the CPU for this time in micro-secs
     */
    static void _setTimerSignal(int quantum_usecs);

    /** Looking for possible thread to be terminated from previous execution context, if exists - deleting it.
     *  Note: it's manually invoked on each library's function. We avoid deleting immediately when requested because
     *  in case of terminating itself, the thread will lead to use-after-free of its stack-allocated memory. */
    void _deleteTerminatedThread();

public:
    // there will be one instance created and the library calls will be forwarded to it
    explicit Scheduler(int quantum_usecs);

    // prohibit copying Scheduler objects
    Scheduler(const Scheduler &) = delete;
    Scheduler operator=(const Scheduler &) = delete;

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
    inline int getTid() const { return currentRunningThread_; }

    /**
     * @return Total number of quantums that occupied the CPU so far
     */
    inline int getTotalQuantums() const { return total_quantum_; }

    /**
     * @param tid Thread ID
     * @return Number of quantums started by the given thread, otherwise -1 in case no such
     * thread
     */
    int getThreadQuantums(int tid);

    /**
     * Blocks a thread, may cause a preemption operation
     * @param tid Thread id to be blocked, mustn't be the main thread (==0)
     * @return 0 on success, else -1 indicating error code
     */
    int blockThread(int tid);

    /**
     * Resumes a thread.
     * @param tid Thread id to be resumed. May or may not be already blocked.
     * @return 0 on success, else -1 indicating error code
     */
    int resumeThread(int tid);

    /**
     * Blocks the currently running thread to #`num_quantums` quantums.
     * @param tid Thread id to be resumed. May or may not be already blocked.
     * @return 0 on success, else -1 indicating error code
     */
    int sleepThread(int num_quantums);

    /**
     * This method is in charge of preempting the running thread, scheduling the next one
     * to occupy the CPU.
     * @param signo Here only because of sa_handler function's signature
     */
    void timerHandler(int signo);

    /**
     * Checks on every quantum whether there are threads that are sleeping and can be wakened up. If there are - it
     * transitions them into the ready queue, such that they can race with other ready threads at the next turns.
     */
    void _sleepToReady();

    std::multiset<TidToSleepTime, sleepTimeCmp>::iterator _isTidSleeping(int tid) const;

    int _getSpawnedThreadReady(threadEntryPoint function, int nextTid);

    void _applySigJmp(const PreemptReason &preemptReason, int nextTid);

    void _setTimer();

    void _terminateOtherThread(int tid);

    int _blockOtherThread(int tid);
};

/**
 * Wrapper for _timeHandler function, as we can't pass function members as parameters
 * @param signo The ID of the timer signal.
 */
void timerHandlerGlobal(int signo);

#endif // PROJECT_EX2_SCHEDULER_H
