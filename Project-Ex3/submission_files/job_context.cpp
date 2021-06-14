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
  jobStateMutex_(PTHREAD_MUTEX_INITIALIZER),
  outputVecMutex_(PTHREAD_MUTEX_INITIALIZER)
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

void JobContext::lockJobStateMutex()
{
    if (pthread_mutex_lock(&jobStateMutex_))
    {
        systemError("[[pthread_mutex_lock]] failed.");
    }
}

void JobContext::unlockJobStateMutex()
{
    if (pthread_mutex_unlock(&jobStateMutex_))
    {
        systemError("[[pthread_mutex_unlock]] failed.");
    }
}

void JobContext::lockOutputVecMutex()
{
    if (pthread_mutex_lock(&outputVecMutex_))
    {
        systemError("[[pthread_mutex_lock]] failed.");
    }
}

void JobContext::unlockOutputVecMutex()
{
    if (pthread_mutex_unlock(&outputVecMutex_))
    {
        systemError("[[pthread_mutex_unlock]] failed.");
    }
}

void JobContext::updateOutputVector(OutputPair &&outputPair)
{
    // anyone of the threads may try updating this output vector, hence we have to make this update a critical section
    lockOutputVecMutex();
    outputVec_.emplace_back(outputPair);
    unlockOutputVecMutex();
}