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
  threadsBarrier_(numOfThreads_),
  pthreadMutex_(PTHREAD_MUTEX_INITIALIZER)
{
  for (size_t i = 0; i < numOfThreads_ - 1; ++i)
  {
      // construct each of them
      threadContexts_.emplace_back(new ThreadContext(i, *this));
  }
}

void JobContext::getJobDone()
{
    // TODO - It is legal to call the function more than once and we should handle it
    for (size_t i = 0; i < numOfThreads_ - 1; ++i)
    {
        threadContexts_[i]->pthreadJoin();
    }
}

void JobContext::lockMutex()
{
    if (pthread_mutex_lock(&pthreadMutex_))
    {
        systemError("[[pthread_mutex_lock]] failed.");
    }
}

void JobContext::unlockMutex()
{
    if (pthread_mutex_unlock(&pthreadMutex_))
    {
        systemError("[[pthread_mutex_unlock]] failed.");
    }
}

void JobContext::updateOutputVector(OutputPair &&outputPair)
{
    outputVec_.emplace_back(outputPair);
}