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
    *state = jobContext->getJobState();
}

void emit2(K2 *key, V2 *value, void *context)
{
    auto threadContext = static_cast<ThreadContext*>(context);
    threadContext->pushIntermediateElem(IntermediatePair(key, value));
}

void emit3(K3* key, V3* value, void* context)
{
    auto threadContext = static_cast<ThreadContext*>(context);
    threadContext->pushOutputElem(OutputPair(key, value));
}

void closeJobHandle(JobHandle job)
{
    waitForJob(job);
    auto jobToDelete = static_cast<JobContext*>(job);
    delete jobToDelete;
}