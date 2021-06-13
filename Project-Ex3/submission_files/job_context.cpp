//
// Created by dan-os on 09/06/2021.
//

#include "job_context.h"

JobContext::JobContext(const MapReduceClient &client, const InputVec &inputVec, OutputVec &outputVec, int numOfThreads,
                       void *(*threadEntryPoint)(void *))
: client_(client),
  inputVec_(inputVec),
  outputVec_(outputVec),
  currJobState_({UNDEFINED_STAGE}),
  numOfThreads_(numOfThreads),
  threadContexts_(numOfThreads), // TODO is it necessary?
  lastThreadWorker_(0),
  threadsBarrier_(numOfThreads_)
{
  for (size_t i = 0; i < numOfThreads_ - 1; ++i)
  {
      // construct each of them - ThreadContext ctor is implicitly called here
      threadContexts_.emplace_back(i, *this, threadEntryPoint);
  }
}

void JobContext::getJobDone()
{
    for (size_t i = 0; i < numOfThreads_ - 1; ++i)
    {
        threadContexts_.pthreadJoin();
    }
}