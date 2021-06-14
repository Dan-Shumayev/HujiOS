//
// Created by dan-os on 12/06/2021.
//

#include "thread_context.h"

ThreadContext:ThreadContext(size_t tid, JobContext& jobContext)
: pthread_thread_id_(tid),
  currentJobContext_(jobContext)
{
    if (pthread_create(&pthreadThread_, nullptr, threadEntryPoint, static_cast<void*>(this)) != 0)
    {
        mapReduceLibraryError("[[pthread_create]] failed.");
    }
}

void ThreadContext::pthreadJoin()
{
    if (pthread_join(pthreadThread_, nullptr) != 0)
    {
        mapReduceLibraryError("[[pthread_join]] failed.");
    }
}

void ThreadContext::invokeMapPhase()
{
    JobContext& currJobContext = currentJobContext_;
    size_t ix;
    while ((ix = currJobContext.lastThreadAtomicGetIncrement()) < currJobContext.getNumOfInputElems())
    {
        // distributing input elements among the thread workers
        auto elem = currJobContext.getInputVector()[ix];
        currJobContext.invokeClientMapRoutine(elem.first, elem.second, static_cast<void *>(this));
    }
    // TODO percentage, job state, etc. accordingly - to be continued...
}

void ThreadContext::invokeSortPhase()
{
    std::sort(intermediateVec_.begin(), intermediateVec_.end());
    // TODO - ensure we don't need nothing else here
}

void ThreadContext::invokeShufflePhase()
{
    // TODO - implement logic
}

void ThreadContext::invokeReducePhase()
{
    // TODO - implement logic
}