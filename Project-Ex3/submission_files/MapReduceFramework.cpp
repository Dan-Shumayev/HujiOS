//
// Created by dan-os on 09/06/2021.
//

#include "job_context.h" // MapReduceFrameworkJobContext, exceptions, ThreadContext, barrier
#include <algorithm>

JobHandle startMapReduceJob(const MapReduceClient &client, const InputVec &inputVec,
                  OutputVec &outputVec, int multiThreadLevel)
{
    auto job = new JobContext(client, inputVec, outputVec, multiThreadLevel);
    return static_cast<JobHandle>(job); // return jobHandle to client
}

void waitForJob(JobHandle job)
{
    auto jobContext = static_cast<JobContext*>(job);
    jobContext->getJobDone();
}

void getJobState(JobHandle job, JobState *state)
{
    auto jobContext = static_cast<JobContext *>(job);
    jobContext->lockJobStateMutex();

    *state = jobContext->getJobState();// TODO - valid and efficient struct copy-assignment?

    jobContext->unlockJobStateMutex();
}

void emit2(K2 *key, V2 *value, void *context)
{
    auto threadContext = static_cast<ThreadContext*>(context);
    // TODO - do we need some mutex-locking here? "the function updates the number of intermediary elements using
    //  atomic counter" - what do they mean by that? which counter?
    threadContext->pushIntermediateElem(IntermediatePair(key, value));
}

void emit3(K3* key, V3* value, void* context)
{
    auto threadContext = static_cast<ThreadContext*>(context);
    threadContext->pushOutputElem(OutputPair(key, value));
}