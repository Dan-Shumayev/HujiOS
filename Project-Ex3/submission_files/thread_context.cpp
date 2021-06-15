//
// Created by dan-os on 12/06/2021.
//

#include <unistd.h>
#include <unordered_map>
#include "thread_context.h"
#include "job_context.h"


ThreadContext::ThreadContext(size_t tid, JobContext& jobContext)
: thread_id_(tid),
  pthreadThread_(),
  currentJobContext_(jobContext),
  isJoined_(false)
{
    if (pthread_create(&pthreadThread_, nullptr, _threadEntryPoint, static_cast<void *>(this)))
    {
        systemError("[[pthread_create]] failed.");
    }
}

void *ThreadContext::_threadEntryPoint(void *context)
{
    // Setup threadContext and jobContext
    auto threadContext = static_cast<ThreadContext*>(context);
    JobContext& currJobContext = threadContext->getJobContext();

    /** Map phase */
    threadContext->invokeMapPhase();

    /** Sort phase */
    threadContext->invokeSortPhase();

    /** Barrier - Let's wait here until all threads finish mapping and sorting */
    currJobContext.barrier();

    /** Shuffle phase */
    threadContext->invokeShufflePhase();

    /** Barrier -  Let all threads wait here until thread 0 finishes shuffling */
    currJobContext.barrier();

    /** Reduce phase */
    threadContext->invokeReducePhase();

    return nullptr; // we must(as per-pthread_create) return something as we promise returning void-pointer, hence null
}

void ThreadContext::pthreadJoin()
{
    if (!isJoined_ && pthread_join(pthreadThread_, nullptr))
    {
        systemError("[[pthread_join]] failed.");
    }
    isJoined_ = true;
}

void ThreadContext::invokeMapPhase()
{
    currentJobContext_.setJobStateStage(MAP_STAGE);
    size_t numOfInputElems = currentJobContext_.getNumOfInputElems();
    size_t ix;
    while ((ix = currentJobContext_.lastThreadAtomicGetIncrement()) < numOfInputElems)
    {
        // distributing input elements among the thread workers
        auto elem = currentJobContext_.getInputVector()[ix];
        currentJobContext_.invokeClientMapRoutine(elem.first, elem.second, static_cast<void *>(this));

        auto amountOfProcessedElems = currentJobContext_.lastProcessedInputElementGetAndIncrement();
        currentJobContext_.setJobStatePercentage((float(amountOfProcessedElems) / numOfInputElems) * 100);
    }
}

void ThreadContext::invokeSortPhase()
{
    std::sort(intermediateVec_.begin(), intermediateVec_.end()); // Elements are compared using operator<
}

void ThreadContext::invokeShufflePhase()
{
    if (thread_id_ == 0) // by convention - only thread-0 performs the shuffle phase
    {
        // TODO - do this function from scratch!
        currentJobContext_.setJobStateStage(SHUFFLE_STAGE);
        size_t numOfIntermediatePairs = currentJobContext_.getNumOfIntermediatePairs();

        auto numOfThreads = currentJobContext_.getNumOfThreads();
        //IntermediatePair==std::pair<K2*, V2*>
        std::unordered_map<K2*, IntermediateVec> keyToVectorOfPairs;//IntermediateVec==std::vector<IntermediatePair>
        for (size_t i = 0; i < numOfThreads; ++i){
            // std::vector<std::pair<K2*, V2*>> sorted by K2
            auto currIntermediateVec =
                    currentJobContext_.getThreadContext(i).getIntermediateVec();
            for(const auto& elem : currIntermediateVec){
                keyToVectorOfPairs[elem.first].emplace_back(elem);
            }
        }
        auto shuffledQueue = currentJobContext_.getShuffledQueue(); // std::vector<std::vector<std::pair<K2*, V2*>>>
        for(const auto& sameKeyPairVector : keyToVectorOfPairs){
            shuffledQueue.emplace_back(sameKeyPairVector.second);
        }

        currentJobContext_.setJobStatePercentage(
                (float(JobContext::getNumOfShuffledPairs(
                        shuffledQueue)) / numOfIntermediatePairs) * 100);
    }
    // else => you aren't thread-0, so go wait on barrier
}

void ThreadContext::invokeReducePhase()
{
    currentJobContext_.setJobStateStage(REDUCE_STAGE);
    size_t numOfShuffleElems = JobContext::getNumOfShuffledPairs(currentJobContext_.getShuffledQueue());
    size_t ix;
    auto shuffleQueue = currentJobContext_.getShuffledQueue();
    while ((ix = currentJobContext_.shuffleAtomicCounter()) < shuffleQueue.size())
    {
        currentJobContext_.invokeClientReduceRoutine(&shuffleQueue[ix],static_cast<void *>(this));

        auto amountOfProcessedElems = currentJobContext_.lastProcessedShuffledElementGetAndIncrement();
        currentJobContext_.setJobStatePercentage(
                (float(amountOfProcessedElems) / numOfShuffleElems) * 100);
    }
}

void ThreadContext::pushIntermediateElem(IntermediatePair&& intermediatePair)
{
    intermediateVec_.emplace_back(intermediatePair);
}

void ThreadContext::pushOutputElem(OutputPair &&outputPair)
{
    currentJobContext_.updateOutputVector(std::forward<OutputPair>(outputPair));
}