//
// Created by dan-os on 12/06/2021.
//

#include <unistd.h>
#include "thread_context.h"
#include "job_context.h"


ThreadContext::ThreadContext(size_t tid, JobContext& jobContext)
: thread_id_(tid),
  pthreadThread_(),
  currentJobContext_(jobContext)
{
    if (pthread_create(&pthreadThread_, nullptr, _threadEntryPoint, static_cast<void *>(this)))
    {
        systemError("[[pthread_create]] failed.");
    }
}

void *ThreadContext::_threadEntryPoint(void *context)
{
    // Setup threadContext and jobContext
    auto threadContext = static_cast<ThreadContext*>(context);
    JobContext& currJobContext = threadContext->getJobContext();

    /** Map phase */
    threadContext->invokeMapPhase();

    /** Sort phase */
    threadContext->invokeSortPhase();

    /** Barrier - Let's wait here until all threads finish mapping and sorting */
    currJobContext.barrier();

    /** Shuffle phase */
    threadContext->invokeShufflePhase();

    /** Barrier -  Let all threads wait here until thread 0 finishes shuffling */
    currJobContext.barrier();

    /** Reduce phase */
    threadContext->invokeReducePhase();

    return nullptr; // we must(as per-pthread_create) return something as we promise returning void-pointer, hence null
}

void ThreadContext::pthreadJoin()
{
    if (pthread_join(pthreadThread_, nullptr))
    {
        systemError("[[pthread_join]] failed.");
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
    std::sort(intermediateVec_.begin(), intermediateVec_.end()); // Elements are compared using operator<
}

void ThreadContext::invokeShufflePhase()
{
    if (thread_id_ == 0) // by convention - only thread-0 performs the shuffle phase
    {

    }
    else
    {
        // if you're not thread-0 => get some sleep letting him finish shuffling
        sleep(1);
    }
}

void ThreadContext::invokeReducePhase()
{
    // TODO - implement logic
}

void ThreadContext::pushIntermediateElem(IntermediatePair&& intermediatePair)
{
    intermediateVec_.emplace_back(intermediatePair);
}

void ThreadContext::pushOutputElem(OutputPair &&outputPair)
{
    currentJobContext_.updateOutputVector(std::forward<OutputPair>(outputPair));
}