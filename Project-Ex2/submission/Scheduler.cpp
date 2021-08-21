//
// Created by dan-os on 31/05/2021.
//

#include "Scheduler.h"
#include "uthreads.h" // MAX_THREAD_NUM
#include "thread.h" // Thread object
#include <utility> // std::piecewise_construct
#include <tuple> // std::forward_as_tuple
#include <cstdlib> // std::exit

Scheduler::Scheduler(int quantum_usecs) // map, deque, set and struct are default constructed implicitly
: currentRunningThread_(0), // main thread(0) initializes the scheduler
    tidToTerminate_(-1), // -1 indicates no thread is supposed to be terminated
    total_quantum_(0),
    sigAlarm_(),
    mutexLockedByThreadId_(-1),
  threadQuantum_(quantum_usecs)
{
    /** Insert the main thread into the entire collection of concurrent threads,
    Assuming no element with this ID in threads_. using piecewise_construct because it's not copyable
    so it constructs the pair manually.
    Note: the thread element representing the main thread is default-constructed. */
    threads_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(0),
                     std::forward_as_tuple());

    Thread& main = threads_[0];
    main.incrementNumOfQuantum(); // First quantum occupied by the main thread
    total_quantum_++; // Total quantum

    // set timer handler
//    struct sigaction sigAlarm_{}; // Initialize all the fields
    sigAlarm_.sa_handler = timerHandlerGlobal; // Assign the first field of sigAlarm (sa_handler) as needed, others zeroed
    if (sigaction(SIGVTALRM, &sigAlarm_, nullptr) != 0)
    {
        uthreadSystemException("sigaction");
    }

    // setup timer signal every quantum_usecs micro-secs for all threads (including main thread)
    _setTimerSignal(threadQuantum_);
}

/** Private (internal purposes) methods */

void Scheduler::_setTimerSignal(int quantum_usecs)
{
    itimerval timer = { {0, quantum_usecs}, {0, quantum_usecs} };
    if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) != 0)
    {
        uthreadSystemException("setittimer");
    }
}

int Scheduler::_getLowestAvailableId() const
{
    for (int tid = 1; tid <= (int)threads_.size(); ++tid)
    {
        if (threads_.find(tid) == threads_.end()) // Indicating no such element (thread with this tid)
        {
            return tid;
        }
    }

    return EXIT_FAIL;
}

bool Scheduler::_isThreadExist(int tid)
{
    if (threads_.find(tid) == threads_.end()) // Doesn't exist
    {
        return false;
    }
    return true;
}

void Scheduler::_preempt(PreemptReason preemptReason)
{
    // In case its timer expired, so it'll be pushed to end of ready queue
    int preemptedThreadId = currentRunningThread_;

    // Assumption: the readyQueue_ is never empty
    Thread& nextThread = threads_[readyQueue_.front()]; // By-reference, avoiding copy-ctr
    readyQueue_.pop_front();

    currentRunningThread_ = nextThread.get_id();
    ++total_quantum_;
    nextThread.incrementNumOfQuantum();

    if (preemptedThreadId == nextThread.get_id()) // In case only main thread exists,
                                                    // and no other thread spawned so far
    {
        // No need to jump (siglongjmp), because given this branch it implies we never jumped from main thread
        // to another one, so main thread's env is zero (either way, it's an optimization)
        return; // Let the main thread resume its execution
    }

    // "reset" the timer for the newly resumed thread
    // note that all callers to '_preempt' are masked
    // so the following timer can't interrupt us during this
    // routine until we jump
    _setTimerSignal(threadQuantum_);

    if (preemptReason == PreemptReason::Termination)
    {
        tidToTerminate_ = preemptedThreadId;
        // the thread being terminated will be deleted from memory only once
        // we have jumped to the next thread, using the uthread library's functions
        siglongjmp(nextThread.get_env(), 1);
    }
    else // Quantum expired OR Blocked
    {
        Thread& previousThread = threads_[preemptedThreadId]; // assumed that there's a thread with this ID
        if (sigsetjmp(previousThread.get_env(), 1) != 0)
        {
            // if we're here, we jumped back to previousThread, so let's resume its execution
            // from its last instruction stored by env
            return;
        }
        siglongjmp(nextThread.get_env(), 1); // jump to the next thread
    }
}

void Scheduler::_deleteReadyThread(int tid)
{
    for (auto threadIt = readyQueue_.begin(); threadIt != readyQueue_.end(); ++threadIt)
    {
        if (*threadIt == tid) // iterator de-reference
        {
            readyQueue_.erase(threadIt);
        }
    }
}

void Scheduler::_deleteTerminatedThread() {
    if (tidToTerminate_ != -1) // is there a thread to terminate?
    {
        threads_.erase(tidToTerminate_);
        tidToTerminate_ = -1; // nothing to erase for now
    }
}

/** API methods */

void Scheduler::timerHandler(int signo)
{
    _deleteTerminatedThread();
    if (signo != SIGVTALRM)
    {
        uthreadException("Not the virtual timer signal. SIGVTALRM is required.");
        return;
    }
    readyQueue_.emplace_back(currentRunningThread_);
    _preempt(PreemptReason::QuantumExpiration);
}

int Scheduler::spawnThread(threadEntryPoint function)
{
    _deleteTerminatedThread();
    if (function == nullptr)
    {
        return uthreadException("Can't spawn thread with nullptr function");
    }
    if (threads_.size() == MAX_THREAD_NUM)
    {
        return uthreadException("Maximum thread count was reached");
    }

    int nextTid = _getLowestAvailableId();
    if (nextTid == EXIT_FAIL)
    {
        return EXIT_FAIL;
    }
    /** Insert the newly spawned thread into the entire collection of concurrent threads,
        Assuming no element with this ID in threads_. using some trickery because it's not copyable*/
    threads_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(nextTid),
                     std::forward_as_tuple(nextTid, function));
    readyQueue_.emplace_back(nextTid); // Inserted to the end of ready threads

    return nextTid;
}

int Scheduler::terminateThread(int tid)
{
    _deleteTerminatedThread();
    if (tid == 0) // Main thread
    {
        // since Scheduler's only instance is via a static unique_ptr in main thread (scheduler_manager),
        // the static variable will be destroyed (per the CPP standard) and
        // thus so will Scheduler and all its resources (threads).
        std::exit(EXIT_SUCCESS);
    }
    if (!_isThreadExist(tid))
    {
        return uthreadException("Don't terminate a non existent thread");
    }
    else if (tid == currentRunningThread_)
    {
        _preempt(PreemptReason::Termination);
    }
    else // The thread is blocked/ready - terminate it by only delete it from suitable data structures
    {
        if (blockedThreads_.erase(tid) == 0 && blockedByMutexThreads_.erase(tid) == 0)
            // not in blocked threads => then in ready
        {
            _deleteReadyThread(tid);
        }
    }
    return EXIT_SUCCESS;
}

int Scheduler::getThreadQuantums(int tid)
{
    _deleteTerminatedThread();
    if (!_isThreadExist(tid) || tidToTerminate_ == tid)
    {
        return uthreadException("Can't get quantums of non existent thread");
    }
    Thread& thread = threads_[tid];
    return thread.get_quantum_running();
}

int Scheduler::blockThread(int tid)
{
    _deleteTerminatedThread();
    if (!_isThreadExist(tid))
    {
        return uthreadException("Don't block non existent thread");
    }
    if (tid == 0) // main thread can't be blocked
    {
        return uthreadException("Can't block main thread");
    }
//    Thread& threadToBlock = threads_[tid]; // TODO - consider using it
    if (blockedThreads_.find(tid) != blockedThreads_.end()) // already blocked -> no-operation required
    {
        return EXIT_SUCCESS;
    }
    if (currentRunningThread_ == tid)
    {
        blockedThreads_.emplace(tid);
        _preempt(PreemptReason::Blocking); // block the running thread
    }
    else // in ready queue
    {
        blockedThreads_.emplace(tid);

        // delete this thread from the ready queue
        _deleteReadyThread(tid);
    }
    return EXIT_SUCCESS;
}

int Scheduler::resumeThread(int tid)
{
    _deleteTerminatedThread();
    if (!_isThreadExist(tid))
    {
        return uthreadException("Can't resume non existent thread");
    }
    auto threadIterator = blockedThreads_.find(tid); // store it in case we'll erase it from blocked set
    if (threadIterator == blockedThreads_.end()) // not blocked thread => no-operation
    {
        return EXIT_SUCCESS;
    }
    auto threadIt = blockedByMutexThreads_.find(tid);
    if (threadIt == blockedByMutexThreads_.end())
    {
        // it's not mutex-blocked, then delete it from blocked threads and queue it to the ready threads
        auto threadIt = blockedThreads_.find(tid);
        blockedThreads_.erase(threadIt);
        readyQueue_.emplace_back(tid);
    }
    // is it mutex-blocked? if it's, remain unblocked
    return EXIT_SUCCESS;
}

int Scheduler::mutexTryLock()
{
    _deleteTerminatedThread();
    if (mutexLockedByThreadId_ == -1) // available mutex
    {
        mutexLockedByThreadId_ = currentRunningThread_;
        return EXIT_SUCCESS;
    }
    // locked
    if (mutexLockedByThreadId_ == currentRunningThread_) // the thread locking the mutex wants to re-lock?
    {
        return uthreadException("the thread locking the mutex wants to re-lock it");
    }

    // here the thread has to be blocked because of the mutex locked
    blockedByMutexThreads_.emplace(currentRunningThread_); // it may be called again upon mutex unlocking
    blockThread(currentRunningThread_);

    return EXIT_SUCCESS;
}

int Scheduler::mutexTryUnlock()
{
    _deleteTerminatedThread();
    if (mutexLockedByThreadId_ == -1) // already unlocked
    {
        return uthreadException("can't unlock unlocked mutex");
    }

    mutexLockedByThreadId_ = -1; // mutex unlocked
    if (!blockedByMutexThreads_.empty()) // Is there any blocked-by-mutex-thread?
    {
        int threadIdToLockMutex = *(blockedByMutexThreads_.begin()); // pick an arbitrary thread to lock the mutex
        resumeThread(threadIdToLockMutex); /** TODO - here we have a mistake. we have to ensure that our
                                                mutex-blocked thread isn't blocked by uthread_block.
                                                if it's, then we won't resume it.
                                                blockedbythread / blockedbymutex / blockedbyboth.
                                                we release a thread:
                                                blockedbythread->unblockbythread /
                                                blockedbymutex->unlockmutex /
                                                blockedbyboth->has to be resumed by mutex-unlock and by thread*/
        blockedThreads_.erase(threadIdToLockMutex);
        blockedByMutexThreads_.erase(threadIdToLockMutex);
    }
    return EXIT_SUCCESS;
}