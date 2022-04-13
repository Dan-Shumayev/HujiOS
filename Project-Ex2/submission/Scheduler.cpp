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
    total_quantum_++;             // Total quantum

    /** Insert the main thread into the entire collection of concurrent threads,
    Assuming no element with this ID in threads_. using piecewise_construct because it's not copyable
    so it constructs the pair manually.
    Note: the thread element representing the main thread is default-constructed. */
    threads_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(0),
                     std::forward_as_tuple());
    threads_[0].incrementNumOfQuantum(); // First quantum occupied by the main thread

    // setup timer handler, and signal every quantum_usecs micro-secs for all threads (including the main thread)
    _setTimer();
}

/** Private (internal purposes) methods */

void Scheduler::_setTimer()
{
    // set timer handler up
    sigAlarm_.sa_handler = timerHandlerGlobal; // Assign the first field of sigAlarm (sa_handler) as needed, others zeroed
    if (sigaction(SIGVTALRM, &sigAlarm_, nullptr) != 0)
    {
        uthreadSystemException("sigaction");
    }

    _setTimerSignal(threadQuantum_);
}

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
    size_t lowestTid = 1;
    while (lowestTid < MAX_THREAD_NUM && threads_.find(lowestTid) != threads_.end())
    {
        lowestTid++;
    }

    return lowestTid == MAX_THREAD_NUM ? EXIT_FAIL : (int)lowestTid;
}

bool Scheduler::_isThreadExist(int tid)
{
    return !(threads_.find(tid) == threads_.end());
}

void Scheduler::_preempt(PreemptReason preemptReason)
{
    auto nextTid = readyQueue_.front();
    readyQueue_.pop_front();

    ++total_quantum_;
    threads_[nextTid].incrementNumOfQuantum();

    if (currentRunningThread_ == nextTid) // In case only main thread exists,
        // and no other thread spawned so far
    {

        // No need to jump (siglongjmp), because given this branch it implies we never jumped from main thread
        // to another one, so main thread's env is zero (either way, it's an optimization)
        return; // Let the main thread resume its execution
    }

    _applySigJmp(preemptReason, nextTid);
}

void Scheduler::_applySigJmp(const Scheduler::PreemptReason &preemptReason, int nextTid)
{
    // In case its timer expired, so it'll be pushed to end of ready queue
    int preemptedThreadId = currentRunningThread_;

    // Assumption: the readyQueue_ is never empty
    Thread &nextThread = threads_[nextTid]; // By-reference, avoiding copy-ctr
    currentRunningThread_ = nextThread.get_id();

    if (preemptReason == PreemptReason::Termination)
    {
        tidToTerminate_ = preemptedThreadId;

        // "reset" the timer for the newly resumed thread
        // note that all callers to '_preempt' are masked
        // so the following timer can't interrupt us during this
        // routine until we jump
        _setTimerSignal(threadQuantum_);
        // the thread being terminated will be deleted from memory only once
        // we have jumped to the next thread, using the uthread library's functions
        siglongjmp(nextThread.get_env(), 1);
    }
    else // Quantum expired OR Blocked OR Falls asleep
    {
        Thread &previousThread = threads_[preemptedThreadId]; // assumed that there's a thread with this ID
        if (sigsetjmp(previousThread.get_env(), 1) != 0)
        {
            // if we're here, we jumped back to previousThread, so let's resume its execution
            // from its last instruction stored by env
            return;
        }

        // "reset" the timer for the newly resumed thread
        // note that all callers to '_preempt' are masked
        // so the following timer can't interrupt us during this
        // routine until we jump
        _setTimerSignal(threadQuantum_);
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
    if (threads_.erase(tidToTerminate_) > 0) // is there a thread to terminate?
    {
        tidToTerminate_ = -1; // mark as terminated
    }
}

int Scheduler::_blockOtherThread(int tid)
{
    auto it = _isTidSleeping(tid);
    if (it != sleepThreads_.end())
    {
        blockedThreads_.emplace(tid);
        sleepThreads_.erase(it);
    }
    else // in ready queue
    {
        // delete this thread from the ready queue, and block it
        _deleteReadyThread(tid);
        blockedThreads_.emplace(tid);
    }

    return EXIT_SUCCESS;
}

void Scheduler::_sleepToReady()
{
    auto end = sleepThreads_.lower_bound({0, total_quantum_ + 1});
    for(auto it = sleepThreads_.begin(); it != end; ++it)
    {
        readyQueue_.emplace_back(it->first);
    }
    sleepThreads_.erase(sleepThreads_.begin(), end);
}

int Scheduler::_getSpawnedThreadReady(threadEntryPoint function, int nextTid)
{
    readyQueue_.emplace_back(nextTid); // Inserted to the end of ready threads

    /** Insert the newly spawned thread into the entire collection of concurrent threads,
        Assuming no element with this ID in threads_. using some trickery because it's not copyable */
    threads_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(nextTid),
                     std::forward_as_tuple(nextTid, function));
    return nextTid;
}

void Scheduler::_terminateOtherThread(int tid)
{
    tidToTerminate_ = tid;

    auto it = _isTidSleeping(tid);

    if (it == sleepThreads_.end()) // If sleeping and blocked, then it'd be only in blocked
    {
        if (blockedThreads_.erase(tid) < 1) // No blocked one erased -> ready
        {
            _deleteReadyThread(tid);
        }
    }
    else
    {
        // then is sleeping, and can't be blocked. otherwise, it'd be only in blocked as per our impl.
        sleepThreads_.erase(it);
    }
}

std::multiset<TidToSleepTime, sleepTimeCmp>::iterator Scheduler::_isTidSleeping(int tid) const
{
    return std::find_if(sleepThreads_.begin(), sleepThreads_.end(), [tid](const TidToSleepTime& p)
    { return p.first == tid; } );
}


/** API methods */

void Scheduler::timerHandler(int signo)
{
    if (signo != SIGVTALRM)
    {
        uthreadException("Not the virtual timer signal. SIGVTALRM is required.");
        return;
    }

    _deleteTerminatedThread();

    _sleepToReady();

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

    return _getSpawnedThreadReady(function, nextTid);
}

int Scheduler::terminateThread(int tid)
{
    _deleteTerminatedThread();

    if (tid == 0) // Main thread
    {
        // since Scheduler's only instance is via a static unique_ptr in main thread (scheduler_manager),
        // the static variable will be destroyed (as per the CPP standard) and
        // thus so will Scheduler and all its resources (threads).
        std::exit(EXIT_SUCCESS);
    }

    if (!_isThreadExist(tid))
    {
        return uthreadException("Don't terminate a non existent thread");
    }

    if (tid == currentRunningThread_) // Self-terminating
    {
        _preempt(PreemptReason::Termination);
    }
    else // The thread is sleeping/blocked/ready - terminate it by only deleting it from suitable data structures
    {
        _terminateOtherThread(tid);
    }

    return EXIT_SUCCESS;
}

int Scheduler::getThreadQuantums(int tid)
{
    _deleteTerminatedThread();

    if (_isThreadExist(tid) && tid != tidToTerminate_)
    {
        return threads_[tid].get_quantum_running();
    }

    return uthreadException("Can't get quantums of non existent thread");
}

int Scheduler::blockThread(int tid)
{
    _deleteTerminatedThread();

    if (tid == 0) // main thread can't be blocked
    {
        return uthreadException("Can't block main thread");
    }

    if (!_isThreadExist(tid))
    {
        return uthreadException("Don't block non existent thread");
    }

    if (blockedThreads_.find(tid) != blockedThreads_.end()) // already blocked -> no-operation required
    {
        return EXIT_SUCCESS;
    }

    // check if this thread is sleeping, thus dequeue from sleeping and queue into block
    if (currentRunningThread_ == tid) // If sleeping -> can't be currently running
    {
        blockedThreads_.emplace(tid);
        _preempt(PreemptReason::Blocking); // block the running thread

        return EXIT_SUCCESS;
    }

    return _blockOtherThread(tid);
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

    // Insert thread id to sleepThreads iff its sleepUntil != -1, that is, it's was both sleeping and blocked.
    //  It'll be resumed at the next quantums if its sleeping-time has been expired, as our timerHandler transfers on
    //   every quantum the sleeping threads which are to wake up (sleepTime > total_quantums), thus it'll be ready as
    //      requested.
    int sleepUntil = threads_[tid].getSleepUntil();
    if (sleepUntil != -1) // If true -> It was sleeping and blocked, now only sleeping
    {
        if (_isTidSleeping(tid) == sleepThreads_.end())
        {
            sleepThreads_.insert({tid, sleepUntil}); // At the next quantum it'll be ready if sleeping-time expired
        }
    }
    else // Not sleeping, get ready!
    {
        readyQueue_.emplace_back(tid);
    }

    return EXIT_SUCCESS;
}

int Scheduler::sleepThread(int num_quantums)
{
    _deleteTerminatedThread();
    if (currentRunningThread_ == 0) // main current_thread can't be fallen asleep
    {
        return uthreadException("main current_thread can't sleep");
    }
    if (num_quantums <= 0)
    {
        return uthreadException("can't sleep for a non-positive number of quantums");
    }

    Thread& current_thread = threads_[currentRunningThread_];
    current_thread.setSleepUntil(total_quantum_ + num_quantums);
    sleepThreads_.insert({currentRunningThread_, current_thread.getSleepUntil()});
    _preempt((PreemptReason::Blocking));

    return EXIT_SUCCESS;
}