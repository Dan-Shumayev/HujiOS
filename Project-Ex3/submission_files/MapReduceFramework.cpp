//
// Created by dan-os on 09/06/2021.
//

#include "job_context.h" // MapReduceFrameworkJobContext, exceptions, ThreadContext,
                            // barrier
#include <algorithm>

JobHandle startMapReduceJob(const MapReduceClient &client, const InputVec &inputVec,
                  OutputVec &outputVec, int multiThreadLevel)
{
    auto job = new JobContext(client, inputVec, outputVec, multiThreadLevel);
    return static_cast<void*>(job); // return jobHandle to client
}

void waitForJob(JobHandle job)
{
    auto jobContext = static_cast<JobContext*>(job);
    jobContext->getJobDone();
}