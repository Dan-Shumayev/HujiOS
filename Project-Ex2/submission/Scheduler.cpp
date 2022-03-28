//
// Created by dan-os on 31/05/2021.
//

#include "Scheduler.h"
#include "uthreads.h" // MAX_THREAD_NUM
#include "thread.h"   // Thread object
#include <utility>    // std::piecewise_construct
#include <tuple>      // std::forward_as_tuple
#include <cstdlib>    // std::exit
#include <algorithm>  // std::remove, std::find_if

Scheduler::Scheduler(int quantum_usecs) // map, deque, set and struct are default constructed implicitly
    : currentRunningThread_(0),         // main thread(0) initializes the scheduler
      tidToTerminate_(-1),              // -1 indicates no thread is supposed to be terminated
      total_quantum_(0),
      sigAlarm_(),
      threadQuantum_(quantum_usecs)
{
    /** Insert the main thread into the entire collection of concurrent threads,
    Assuming no element with this ID in threads_. using piecewise_construct because it's not copyable
    so it constructs the pair manually.
    Note: the thread element representing the main thread is default-constructed. */
    threads_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(0),
                     std::forward_as_tuple());

    Thread &main = threads_[0];
    main.incrementNumOfQuantum(); // First quantum occupied by the main thread
    total_quantum_++;             // Total quantum

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
    itimerval timer = {{0, quantum_usecs}, {0, quantum_usecs}};
    if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) != 0)
    {
        uthreadSystemException("setittimer");
    }
}

int Scheduler::_getLowestAvailableId() const
{
    for (int tid = 1; tid <= (int)threads_.size(); ++tid)
    {
        if (tid < MAX_THREAD_NUM && threads_.find(tid) == threads_.end()) // Indicating no such element (thread with this tid)
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
    Thread &nextThread = threads_[readyQueue_.front()]; // By-reference, avoiding copy-ctr
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
        Thread &previousThread = threads_[preemptedThreadId]; // assumed that there's a thread with this ID
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
    readyQueue_.erase(
        std::remove(readyQueue_.begin(), readyQueue_.end(), tid),
        readyQueue_.end());
}

void Scheduler::_deleteTerminatedThread()
{
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

    if (!sleepThreads_.empty() && (*sleepThreads_.begin()).second <= total_quantum_) // TODO - to be tested
    {
        auto it = sleepThreads_.begin();
        readyQueue_.emplace_back((*it).first);
        sleepThreads_.erase(it);
    } // TODO - to be tested till here

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
    if (tid == currentRunningThread_)
    {
        _preempt(PreemptReason::Termination);
    }
    else // The thread is sleeping/blocked/ready - terminate it by only deleting it from suitable data structures
    {
        // TODO - to be tested
        threads_.erase(tid);

        auto it = find_if(sleepThreads_.begin(), sleepThreads_.end(), [](const TidToSleepTime & p )
        { return p.first == 0; });

        if (it != sleepThreads_.end())
        {
            sleepThreads_.erase(it);
        }

        if (blockedThreads_.erase(tid) == 0 && it == sleepThreads_.end())
        // not in sleeping/blocked threads => then in ready
        {
            _deleteReadyThread(tid);
        }
        // TODO - to be tested till here
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
    Thread &thread = threads_[tid];
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
    if (blockedThreads_.find(tid) != blockedThreads_.end()) // already blocked -> no-operation required
    {
        return EXIT_SUCCESS;
    }
    // TODO - to be tested from here
    // check if this thread is sleeping, thus dequeue from sleeping and queue into block
    auto it = std::find_if(sleepThreads_.begin(), sleepThreads_.end(), [tid](const TidToSleepTime& p)
    { return p.first == tid; });
    if (it != sleepThreads_.end())
    {
        blockedThreads_.emplace(tid);
        sleepThreads_.erase(it);
    }
    // TODO - to be tested till here
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
    if (threadIterator == blockedThreads_.end())     // not blocked thread => no-operation
    {
        return EXIT_SUCCESS;
    }

    // it's blocked (uthreads_block), then delete it from blocked threads and queue it to the ready threads iff it
    //  doesn't have to sleep
    blockedThreads_.erase(threadIterator);

    // TODO - to be tested
    // insert tid thread to readyQueue_ iff its sleepTime <= currentQuantum, otherwise queue it to the sleepQueue_
    //  which is sorted by each thread's sleepTime
    Thread &currThread = threads_[tid];
    int sleepUntil = currThread.getSleepUntil();
    if (currThread.getSleepUntil() != -1)
    {
        if (sleepUntil <= total_quantum_)
        {
            readyQueue_.emplace_back(tid);
        }
        else
        {
            sleepThreads_.insert({tid, sleepUntil});
        }
    }
    else
    {
        readyQueue_.emplace_back(tid);
    } // TODO - to be tested till here

    return EXIT_SUCCESS;
}

int Scheduler::sleepThread(int num_quantums) // TODO - to be tested
{
    _deleteTerminatedThread();
    if (currentRunningThread_ == 0) // main thread can't be blocked
    {
        return uthreadException("Can't block main thread");
    }

    int sleepUntil = total_quantum_ + num_quantums;
    sleepThreads_.insert({currentRunningThread_, sleepUntil});

    return EXIT_SUCCESS;
}