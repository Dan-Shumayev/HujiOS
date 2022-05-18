#include "job_context.h"

JobContext::JobContext(const MapReduceClient &client, const InputVec &inputVec, OutputVec &outputVec, int numOfThreads)
: client_(client),
  inputVec_(inputVec),
  outputVec_(outputVec),
  jobStateBitwise_(0),
  atomicNextItemToProcess_{},
  numOfThreads_(numOfThreads),
  threadContexts_(numOfThreads),
  threadsBarrier_(numOfThreads),
  jobStateMutex_(PTHREAD_MUTEX_INITIALIZER),
  outputVecMutex_(PTHREAD_MUTEX_INITIALIZER),
  areThreadsJoined_(false),
  pthreadCreateMutex_(PTHREAD_MUTEX_INITIALIZER)
{
    ApplyMutex theLock(getPthreadCreateMutex());

    for (size_t i = 0; i < numOfThreads_; ++i)
    {
        // construct each of them
        threadContexts_[i] = std::unique_ptr<ThreadContext>(new ThreadContext(i, *this));
    }
}

JobContext::~JobContext()
{
    int retval_1 = pthread_mutex_destroy(&jobStateMutex_);
    int retval_2 = pthread_mutex_destroy(&outputVecMutex_);
    int retval_3 = pthread_mutex_destroy(&pthreadCreateMutex_);

    if (retval_1 | retval_2 | retval_3) // Make it so to avoid short-circuiting side effect
    {
        systemError("[[pthread_mutex_destroy]] failed.");
    }
}

void JobContext::getJobDone()
{
    if (!areThreadsJoined_)
    {
        for (size_t i = 0; i < numOfThreads_; ++i)
        {
            threadContexts_[i]->pthreadJoin();
        }
        areThreadsJoined_ = true;
    }
}

void JobContext::updateOutputVector(OutputPair &&outputPair)
{
    // anyone of the threads may try updating this output vector,
    // hence we have to make this update a critical section
    ApplyMutex theLock(outputVecMutex_);
    outputVec_.emplace_back(outputPair);
}

size_t JobContext::getNumOfIntermediatePairs()
{
    size_t threadSize = 0;
    for (size_t i = 0; i < numOfThreads_; ++i)
    {
        threadSize += getThreadContext(i).getIntermediateVec().size();
    }
    return threadSize;
}

void JobContext::setNextPhase()
{
    ApplyMutex theLock(jobStateMutex_);

    uint64_t tmpNextPhase = jobStateBitwise_.load() + 1;
    auto phaseBitsTurnedOn = ((uint64_t)1 << 2) - 1;

    // Atomically turn off all the bits except for the phase bits, which is incremented
    jobStateBitwise_.store(tmpNextPhase & phaseBitsTurnedOn);
}

size_t JobContext::getNextItemToProcess()
{
    return atomicNextItemToProcess_.fetch_add(1);
}

void JobContext::addNumberOfProcessedItems(stage_t currStage, size_t amount)
{
    ApplyMutex theLock(jobStateMutex_);

    auto atomicCurrStage = (stage_t)(jobStateBitwise_ & (uint64_t)3); // Extract the current stage
    if (currStage == atomicCurrStage) {
        jobStateBitwise_.fetch_add((uint64_t) amount << 2);
    }
}

JobState JobContext::getJobState()
{
    ApplyMutex theLock(jobStateMutex_);
    JobState currJobState;

    // Extract the current stage's phase
    auto phaseBitsTurnedOn = ((uint64_t)1 << 2) - 1;
    currJobState.stage = (stage_t)(jobStateBitwise_.load() & phaseBitsTurnedOn);

    // Extract the current number of processed elements
    uint64_t discardPhase = jobStateBitwise_.load() >> 2;
    auto numOfProcessedBitsTurnedOn = ((uint64_t)1 << 32) - 1;
    uint64_t numOfProcessed = discardPhase & numOfProcessedBitsTurnedOn;

    currJobState.percentage = ((float)numOfProcessed / _getTotalItemsOfCurrPhase(currJobState)) * 100;

    return currJobState;
}

float JobContext::_getTotalItemsOfCurrPhase(const JobState &currJobState)
{
    float totalItems = 1.0; // Stale value for UNDEFINED_STAGE
    if (currJobState.stage == MAP_STAGE)
    {
        totalItems = getNumOfInputPairs();
    }
    else if (currJobState.stage == SHUFFLE_STAGE || currJobState.stage == REDUCE_STAGE)
    {
        totalItems = getNumOfIntermediatePairs();
    }

    return totalItems;
}

void JobContext::setJobStage(stage_t currentStage)
{
    ApplyMutex theLock(jobStateMutex_);

    auto atomicCurrStage = (stage_t)(jobStateBitwise_ & (uint64_t)3); // Extract the current stage
    if (currentStage != atomicCurrStage) {
        jobStateBitwise_.store((uint64_t)currentStage);
    }
}

pthread_mutex_t &JobContext::getPthreadCreateMutex() {
    return pthreadCreateMutex_;
}
