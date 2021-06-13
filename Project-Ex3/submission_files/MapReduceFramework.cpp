//
// Created by dan-os on 09/06/2021.
//

#include "MapReduceFramework.h"
#include "Barrier/Barrier.h"
#include "job_context.h"
#include "thread_context.h"
#include "exceptions.h"

#include <pthread.h>
#include <algorithm>

void *threadEntryPoint(void *context)
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
    threadContext->invokeShufflePhase(); // TODO ensure inside this method that only thread-0 will execute Shuffle

    /** Barrier -  Let all threads wait here until thread 0 finishes shuffling */
    currJobContext.barrier();

    /** Reduce phase */
    threadContext->invokeReducePhase();
}

JobHandle startMapReduceJob(const MapReduceClient &client, const InputVec &inputVec,
                  OutputVec &outputVec, int multiThreadLevel)
{
    auto job = new JobContext(client, inputVec, outputVec, multiThreadLevel, threadEntryPoint);
    return static_cast<void*>(job); // return jobHandle to client
}

void waitForJob(JobHandle job)
{
    auto jobContext = static_cast<JobContext*>(job);
    jobContext->getJobDone();
}