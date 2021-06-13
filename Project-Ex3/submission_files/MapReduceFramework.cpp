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
    auto t_cxt = static_cast<ThreadContext*>(context);
    ThreadContext& threadContext = *t_cxt; // TODO - consider using it as a pointer
    JobContext& currJobContext = threadContext.getJobContext();

    /** Map phase */
    threadContext.invokeMapPhase();

    /** Sort phase */
    threadContext.invokeSortPhase();

    /** Barrier - Let's wait here until all threads finish mapping and sorting */
    currJobContext.barrier();

    /** Shuffle phase */
    if (threadContext.getThreadId() == 0)
    {
        threadContext.invokeShufflePhase();
    }

    /** Barrier -  Let all threads wait here until thread 0 finishes shuffling */
    currJobContext.barrier();

    /** Reduce phase */
    threadContext.invokeReducePhase();
}

JobHandle startMapReduceJob(const MapReduceClient &client, const InputVec &inputVec,
                  OutputVec &outputVec, int multiThreadLevel)
{
    auto job = new JobContext(client, inputVec, outputVec, multiThreadLevel); // TODO handle bad_alloc?

    // TODO - move pthread management into ThreadContext
    auto threadWorkers = job->getThreadWorkers(); // pthread_t vector - the actual threads
    auto threadsContext = job->getThreadContexts();
    for (int i = 0; i < multiThreadLevel - 1; i++)
    {
        if (pthread_create(&threadWorkers[i], nullptr, threadEntryPoint, (void*)&threadsContext[i]) != 0)
        {
            mapReduceLibraryError("[[pthread_create]] failed.");
        }
    }
    return static_cast<void*>(job); // return jobHandler to client
}

void waitForJob(JobHandle job)
{
    auto jobContext = static_cast<JobContext*>(job);
    // TODO - move pthread management into ThreadContext
    for (size_t i = 0; i < jobContext->getNumOfThreads(); ++i)
    {
        if (pthread_join(jobContext->getThreadWorkers()[i], nullptr) != 0)
        {
            mapReduceLibraryError("[[pthread_join]] failed.");
        }
    }
}