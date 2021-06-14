//
// Created by dan-os on 09/06/2021.
//

#include "job_context.h"

JobContext::JobContext(const MapReduceClient &client, const InputVec &inputVec, OutputVec &outputVec, int numOfThreads)
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
      // construct each of them
      threadContexts_.emplace_back(new ThreadContext(i, *this));
  }
}

void JobContext::getJobDone()
{
    for (size_t i = 0; i < numOfThreads_ - 1; ++i)
    {
        threadContexts_[i]->pthreadJoin();
    }
}