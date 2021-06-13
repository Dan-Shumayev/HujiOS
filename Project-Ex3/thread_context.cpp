//
// Created by dan-os on 12/06/2021.
//

#include <algorithm>
#include "thread_context.h"

void ThreadContext::invokeMapPhase()
{
    JobContext& currJobContext = currentJobContext_;
    size_t ix;
    while ((ix = currJobContext.lastThreadAtomicGetIncrement()) < currJobContext.getNumOfInputElems())
    {
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