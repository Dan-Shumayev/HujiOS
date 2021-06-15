//
// Created by dan-os on 09/06/2021.
//

#include "job_context.h"

JobContext::JobContext(const MapReduceClient &client, const InputVec &inputVec, OutputVec &outputVec, int numOfThreads)
: client_(client),
  inputVec_(inputVec),
  outputVec_(outputVec),
  currJobState_({UNDEFINED_STAGE}),
  lastProcessedInputElementGetAndIncrement_(0),
  lastProcessedShuffledElementGetAndIncrement_(0),
  jobStateMutex_(PTHREAD_MUTEX_INITIALIZER),
  numOfThreads_(numOfThreads),
  threadContexts_(numOfThreads),
  lastThreadWorker_(0),
  threadsBarrier_(numOfThreads_),
  outputVecMutex_(PTHREAD_MUTEX_INITIALIZER),
  shuffleQueAtomicCounter_(0)
  {
  for (size_t i = 0; i < numOfThreads_ - 1; ++i)
  {
      // construct each of them
      threadContexts_[i] = std::unique_ptr<ThreadContext>(new ThreadContext(i, *this));
  }
}

JobContext::~JobContext()
{
    if (pthread_mutex_destroy(&jobStateMutex_) || pthread_mutex_destroy(&outputVecMutex_))
    {
        systemError("[[pthread_mutex_destroy]] failed.");
    }
}

void JobContext::getJobDone()
{
    for (size_t i = 0; i < numOfThreads_ - 1; ++i)
    {
        threadContexts_[i]->pthreadJoin();
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

size_t JobContext::getNumOfShuffledPairs(std::vector<std::vector<IntermediatePair>> &shuffledQueue)
{
    size_t totalNumOfPairs = 0;
    for (const auto& shuffledVec : shuffledQueue)
    {
        totalNumOfPairs += shuffledVec.size();
    }
    return totalNumOfPairs;
}

size_t JobContext::getNumOfIntermediatePairs()
{
    size_t threadSize;
    for (size_t i = 0; i < numOfThreads_; ++i)
    {
        threadSize = getThreadContext(i).getIntermediateVec().size();
    }
    return threadSize;
}