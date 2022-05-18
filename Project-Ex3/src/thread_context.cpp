#include <unistd.h>
#include <unordered_map>
#include "thread_context.h"
#include "job_context.h"
#include "map_reduce_utils.h"


ThreadContext::ThreadContext(size_t tid, JobContext& jobContext)
: thread_id_(tid),
  pthreadThread_(),
  currentJobContext_(jobContext)
{
    if (pthread_create(&pthreadThread_, nullptr,
                       thread_id_ == 0 ? _threadUniqueEntryPoint : _threadEntryPoint,
                       static_cast<void *>(this)))
    {
        systemError("[[pthread_create]] failed.");
    }
}

void *ThreadContext::_threadUniqueEntryPoint(void *context)
{
    // Setup threadContext and jobContext
    auto threadContext = static_cast<ThreadContext*>(context);
    JobContext& currJobContext = threadContext->_getJobContext();

    /** Map phase */
    threadContext->_invokeMapPhase();

    /** Sort phase */
    threadContext->_invokeSortPhase();

    /** Barrier - Let's wait here until all threads finish mapping and sorting */
    currJobContext.barrier();

    currJobContext.setNextPhase(); // Advance to the next phase, and reset processing info.

    /** Shuffle phase */
    threadContext->_invokeShufflePhase();
    currJobContext.setNextPhase(); // Advance to the next phase, and reset processing info.

    /** Barrier -  Let all threads wait here until thread 0 finishes shuffling */
    currJobContext.barrier();

    /** Reduce phase */
    currJobContext.setJobStage(REDUCE_STAGE);
    threadContext->_invokeReducePhase();

    return nullptr; // we must(as per-pthread_create) return something as we promise returning void-pointer, hence null
}

void *ThreadContext::_threadEntryPoint(void *context)
{
    // Setup threadContext and jobContext
    auto threadContext = static_cast<ThreadContext*>(context);
    JobContext& currJobContext = threadContext->_getJobContext();

    /** Map phase */
    threadContext->_invokeMapPhase();

    /** Sort phase */
    threadContext->_invokeSortPhase();

    /** Barrier - Let's wait here until all threads finish mapping and sorting */
    currJobContext.barrier();
    /** Barrier - Let all threads wait here until thread 0 finishes shuffling */
    currJobContext.barrier();

    /** Reduce phase */
    currJobContext.setJobStage(REDUCE_STAGE);
    threadContext->_invokeReducePhase();

    return nullptr; // we must(as per-pthread_create) return something as we promise returning void-pointer, hence null
}

void ThreadContext::_invokeMapPhase()
{
    // Every thread executes it before getting into the next phase, because they stop at barrier
    //  Then, no matter who sets this when - it remains the same until they all get to barrier
    size_t numOfInputElems = currentJobContext_.getNumOfInputPairs();
    currentJobContext_.setJobStage(MAP_STAGE);

    size_t ix;
    while ((ix = currentJobContext_.getNextItemToProcess()) < numOfInputElems)
    {
        // distributing input elements among the thread workers
        auto elem = currentJobContext_.getInputVector()[ix];
        currentJobContext_.invokeClientMapRoutine(elem.first, elem.second, static_cast<void *>(this));

        currentJobContext_.addNumberOfProcessedItems(MAP_STAGE);
    }
}

void ThreadContext::_invokeSortPhase()
{
    std::sort(intermediateVec_.begin(), intermediateVec_.end(),
              [](const IntermediatePair &lhs, const IntermediatePair &rhs) {
                    return *lhs.first < *rhs.first;
                }
    );
}

void ThreadContext::_invokeShufflePhase()
{
    // TODO - refactor
    ApplyMutex theLock(currentJobContext_.getPthreadCreateMutex());

    currentJobContext_.resetAtomicNextItemToProcess();

    std::vector<IntermediateVec> interVecs;
    for (size_t i = 0; i < currentJobContext_.getNumOfThreads(); ++i)
    {
        interVecs.emplace_back(currentJobContext_.getThreadContext(i).getIntermediateVec());
    }

    auto getMaxKey = []() {
        return [](std::vector<IntermediateVec> &interVecs) {
            K2* maxKey = nullptr;
            for (const auto &v : interVecs) {
                if ((maxKey == nullptr && !v.empty()) || (!v.empty() && (*maxKey < *(v.back().first))))
                    maxKey = v.back().first;
            }
            return maxKey;
        };
    };

    K2* currMaxKey; // Point to the current max key
    auto &shuffledQueue = currentJobContext_.getShuffledQueue(); // std::vector<IntermediateVec>
    while ((currMaxKey = getMaxKey()(interVecs)) != nullptr) // Go over all the keys
    {
        auto newVec = IntermediateVec(); // Current inter vec to be pushed to shuffled
                                            // - with current maximum same key

        for (auto &v : interVecs) // Iterate over each thread's inter-vec
        {
            while (!v.empty() && !( (*(v.back().first) < *currMaxKey) || *currMaxKey < (*(v.back().first)) ) ) // This inter-vec contains this key?
            {
                newVec.push_back(v.back()); // Then push to the current shuffled inter-vec
                v.pop_back();

                currentJobContext_.addNumberOfProcessedItems(SHUFFLE_STAGE);
            }
        }

        shuffledQueue.push_back(newVec); // Push it to our shuffled queue
    }
    currentJobContext_.resetAtomicNextItemToProcess();
}

void ThreadContext::_invokeReducePhase()
{
    auto shuffleQueue = currentJobContext_.getShuffledQueue();

    size_t ix;
    while ((ix = currentJobContext_.getNextItemToProcess()) < shuffleQueue.size())
    {
        currentJobContext_.invokeClientReduceRoutine(&shuffleQueue[ix],static_cast<void *>(this));

        currentJobContext_.addNumberOfProcessedItems(REDUCE_STAGE, shuffleQueue[ix].size());
    }
}

void ThreadContext::pushIntermediateElem(IntermediatePair &&intermediatePair)
{
    intermediateVec_.emplace_back(intermediatePair);
}

void ThreadContext::pushOutputElem(OutputPair &&outputPair)
{
    currentJobContext_.updateOutputVector(std::forward<OutputPair>(outputPair));
}

void ThreadContext::pthreadJoin() const
{
    if (pthread_join(pthreadThread_, nullptr)) {
        systemError("[[pthread_join]] failed.");
    }
}