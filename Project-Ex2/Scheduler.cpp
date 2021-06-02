//
// Created by dan-os on 31/05/2021.
//

#include "Scheduler.h"
#include "uthreads.h" // MAX_THREAD_NUM
#include "thread.h" // Thread object
#include <utility> // std::piecewise_construct
#include <tuple> // std::forward_as_tuple

Scheduler::Scheduler(int quantum_usecs) // map, deque, set and struct are default constructed implicitly
: currentRunningThread_(0), // main thread(0) initializes the scheduler
    tidToTerminate_(-1), // -1 indicates no thread is supposed to be terminated
    total_quantum_(1),
    sigAlarm_{timerHandlerGlobal} // initializes the first field of sigAlarm (sa_handler) as needed, others zeroed
{
    /** Insert the main thread into the entire collection of concurrent threads,
    Assuming no element with this ID in threads_. using piecewise_construct because it's not copyable
    so it constructs the pair manually.
    Note: the thread element representing the main thread is default-constructed. */
    threads_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(0),
                     std::forward_as_tuple());

    Thread& main = threads_[0];
    main.incrementNumOfQuantum();

    // set timer handler
    if (sigaction(SIGVTALRM, &sigAlarm_, nullptr) != 0)
    {
        systemError("sigaction");
    }

    // setup timer signal every quantum_usecs micro-secs for all threads (including main thread)
    _setTimerSignal(quantum_usecs);
}

/** Private (internal purposes) methods */

void Scheduler::_setTimerSignal(int quantum_usecs)
{
    itimerval timer = { {0, quantum_usecs}, {0, quantum_usecs} };
    if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) != 0)
    {
        systemError("setittimer");
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

    return EXIT_FAILURE;
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
    if (preemptionReason == PreemptReason::Termination)
    {
        // TODO - detect by the next execution of the library and delete it there
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
    return;
}

void Scheduler::_deleteTerminatedThread() {
    if (tidToTerminate_ != -1) // is there a thread to terminate?
    {
        threads_.erase(_tidBeingTerminated);
        tidToTerminate_ = -1; // nothing to erase for now
    }
}

/** API methods */

void Scheduler::timerHandler(int signo)
{
    _deleteTerminatedThread();
    if (signo != SIGVTALRM)
    {
        return threadLibraryError("Not the virtual timer signal. SIGVTALRM is required.");
    }
    readyQueue_.emplace_back(currentRunningThread_);
    _preempt(PreemptReason::QuantumExpiration);
}

int Scheduler::spawnThread(threadEntryPoint function)
{
    _deleteTerminatedThread();
    if (function == nullptr)
    {
        return threadLibraryError("Can't spawn thread with nullptr function");
    }
    if (threads_.size() == MAX_THREAD_NUM)
    {
        return threadLibraryError("Maximum thread count was reached");
    }

    int nextTid = _getLowestAvailableId();
    if (nextTid == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    /** Insert the newly spawned thread into the entire collection of concurrent threads,
        Assuming no element with this ID in threads_. using some trickery because it's not copyable*/
    threads_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(nextTid),
                     std::forward_as_tuple(nextTid, function));
    readyQueue_.emplace_back(nextTid); // Inserted to the end of ready threads

    return nextTid;
}

int terminateThread(int tid)
{
    _deleteTerminatedThread();
    if (tid == 0) // Main thread
    {
        // since Scheduler's only instance is via a static unique_ptr (scheduler_manager),
        // the static variable will be destroyed (per the CPP standard) and
        // thus so will Scheduler and all its resources (threads).
        std::exit(EXIT_SUCCESS);
    }
    if (!_isThreadExist(tid))
    {
        return threadLibraryError("Can't terminate non existent thread");
    }
    else if (tid == currentRunningThread_)
    {
        _preempt(PreemptReason::Termination);
    }
    else // The thread is blocked/ready - terminate it by only delete it from suitable data structures
    { // TODO - what if blocked by mutex?
        if (blockedThreads_.erase(tid) == 0) // not in blocked threads => then in ready
        {
            _deleteReadyThread(tid);
        }
    }
    return EXIT_SUCCESS;
}

int Scheduler::getThreadQuantums(int tid)
{
    _deleteTerminatedThread();
    Thread* thread;
    if (!_isThreadExist(tid) || tidToTerminate_ == tid)
    {
        return threadLibraryError("Can't get quantums of non existent thread");
    }
    Thread& thread = threads_[tid];
    return thread.get_quantum_running();
}

int Scheduler::blockThread(int tid)
{
    _deleteTerminatedThread();
    if (!_isThreadExist(tid))
    {
        return threadLibraryError("Can't block non existent thread");
    }
    if (tid == 0) // main thread can't be blocked
    {
        return threadLibraryError("Can't block main thread");
    }
    Thread& threadToBlock = threads_[tid];
    // TODO - if it's already blocked by mutex?
    if (blockedThreads_.find(tid) != blockedThreads_.end()) // already blocked -> no-operation required
    {
        return EXIT_SUCCESS;
    }
    if (currentRunningThread_ == tid)
    {
        _preempt(PreemptReason::Blocking); // block the running thread
    }
    else // in ready queue
    {
        // delete this thread from the ready queue
        _deleteReadyThread(tid);
    }
    blockedThreads_.emplace(tid); // classify this running/ready thread as blocked
    return EXIT_SUCCESS;
}

int Scheduler::resumeThread(int tid)
{
    _deleteTerminatedThread();
    if (!_isThreadExist(tid))
    {
        return threadLibraryError("Can't resume non existent thread");
    }
    auto threadIterator = blockedThreads_.find(tid); // store it in case we'll erase it from blocked set
    if (threadIterator == _blockSet.end()) // not blocked thread => no-operation
    {
        return EXIT_SUCCESS;
    }
    // it's blocked, then delete it from blocked threads and queue it to the ready threads
    blockedThreads_.erase(threadIterator);
    readyQueue_.emplace_back(tid);

    return EXIT_SUCCESS;
}