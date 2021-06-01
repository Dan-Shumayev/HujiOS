//
// Created by dan-os on 31/05/2021.
//

#include "Scheduler.h"
#include "uthreads.h" // MAX_THREAD_NUM
#include "thread.h" // Thread object
#include <utility> // std::piecewise_construct
#include <tuple> // std::forward_as_tuple

Scheduler::Scheduler(int quantum_usecs) // we can omit the map and deque because default constructed
: thread_quantum_(quantum_usecs),
    total_quantum_(1),
    currentRunningThread_(0) // main thread(0) initializes the scheduler
    // TODO - What else should be initialized?
{}

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

void Scheduler::_preempt()
{
    // In case its timer expired, so it'll be pushed to end of ready queue
    int preemptedThreadId = currentRunningThread_;

    // Assumption: the readyQueue_ is never empty
    Thread& nextThread = threads_[readyQueue_.front()]; // By-reference, avoiding copy-ctr
    readyQueue_.pop_front();

    currentRunningThread_ = nextThread.tid_;
    ++total_quantum_;
    nextThread.incrementNumOfQuantum();

    if (preemptedThreadId == newThread.tid_) // In case only main thread exists, and no other thread spawned so far
    {
        // No need to jump (siglongjmp), because upon this case we never jumped from main thread to another one,
        // so main thread's env is zero (either way, it's an optimization)
        return; // Let the main thread resume its execution
    }
    if (reason == ScheduleReason::Terminate) // TODO implement enum class
    {
        // the thread being terminated will be deleted from memory only once
        // we have jumped to the next thread
        siglongjmp(nextThread.env_, 1);
    }
    // TODO - other cases of scheduling
}

int Scheduler::spawnThread(threadEntryPoint function)
{
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
        tidToTerminate_ = tid; /** Store its ID to terminate during the next resumed thread
                                    TODO - how do we ensure the next thread is deleting it? */
        _preempt(); // TODO - Black box meanwhile
    }
    else // The thread is blocked/ready - terminate it!
    {
        // TODO - we'll have also to delete from blocked/ready structures
        threads_.erase(tid);
    }
    return EXIT_SUCCESS;
}

int Scheduler::getThreadQuantums(int tid)
{
    Thread* thread;
    if (!_doesThreadExist(tid, &thread)) /** TODO blackbox meanwhile | need to init thread
                                                maybe masking is irrelevant here - depends on our
                                                Atomicity necessity */
    {
        return threadLibraryError("Can't get quantums of non existent thread");
    }
    return thread->get_quantum_running();
}

int Scheduler::blockThread(int tid)
{
    Thread* thread;
    if (!_doesThreadExist(tid, &thread))
    {
        return threadLibraryError("Can't block non existent thread");
    }
    else if (tid == 0)
    {
        return threadLibraryError("Can't block main thread");
    }

    /** TODO - magic BLOCKING stuff here
    if (_blockSet.find(tid) != _blockSet.end()) {
        // blocking a blocked thread is a no-op
        return EXIT_SUCCESS;
    }
    _blockSet.emplace(tid);
    if (tid == _runThread)
    {
        _reschedule(ScheduleReason::Blocked);
    } else {
        _deleteFromReadyQueue(tid);
    }
    */

    return EXIT_SUCCESS;
}

int Scheduler::resumeThread(int tid)
{
    if (!_doesThreadExist(tid))
    {
        return threadLibraryError("Can't resume non existent thread");
    }

    /** TODO - magic RESUMING stuff here
    auto blockIt = _blockSet.find(tid);
    if (blockIt == _blockSet.end())
    {
        // resuming a thread that isn't blocked is a no-op
        return EXIT_SUCCESS;
    }
    _blockSet.erase(blockIt);
    _readyQueue.emplace_back(tid);
    */

    return EXIT_SUCCESS;
}