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
    JobContext& currJobContext = threadContext->_getJobContext();

    /** Map phase */
    threadContext->_invokeMapPhase();

    /** Sort phase */
    threadContext->_invokeSortPhase();

    /** Barrier - Let's wait here until all threads finish mapping and sorting */
    currJobContext.barrier();

    /** Shuffle phase */
    threadContext->_invokeShufflePhase();

    /** Barrier -  Let all threads wait here until thread 0 finishes shuffling */
    currJobContext.barrier();

    /** Reduce phase */
    threadContext->_invokeReducePhase();

    return nullptr; // we must(as per-pthread_create) return something as we promise returning void-pointer, hence null
}

void ThreadContext::_invokeMapPhase()
{
    // Every thread executes it before getting into the next phase, because they stop at barrier
    //  Then, no matter who sets this when - it remains the same until they all get to barrier
    currentJobContext_.setJobStateStage(MAP_STAGE);
    size_t numOfInputElems = currentJobContext_.getNumOfInputElems();

    size_t ix;
    while ((ix = currentJobContext_.lastThreadAtomicGetIncrement()) < numOfInputElems)
    {
        // distributing input elements among the thread workers
        auto elem = currentJobContext_.getInputVector()[ix];
        currentJobContext_.invokeClientMapRoutine(elem.first, elem.second, static_cast<void *>(this));

        // TODO - stage update
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
    if (thread_id_ == 0) // W.L.O.G - only thread-0 performs the shuffle phase
    {
        auto numOfIntermediateVecs = currentJobContext_.getNumOfThreads();
        auto totalNumOfInterPairs = currentJobContext_.getNumOfIntermediatePairs();

        currentJobContext_.setJobStateStage(SHUFFLE_STAGE);
        _setPhasePercentage(0.0, numOfIntermediateVecs);

        std::vector<IntermediateVec> interVecs;
        for (size_t i = 0; i < numOfIntermediateVecs; ++i)
        {
            interVecs.emplace_back(currentJobContext_.getThreadContext(i).getIntermediateVec());
        }

        K2* currMaxKey; // Point to the current max key
        auto &shuffledQueue = currentJobContext_.getShuffledQueue(); // std::vector<IntermediateVec>

        auto getMaxKey = []() {
            return [](std::vector<IntermediateVec> &interVecs) {
                K2* maxKey = nullptr;
                for (const auto &v : interVecs){
                    if ((maxKey == nullptr && !v.empty()) || (!v.empty() && (*maxKey < *(v.back().first))))
                        maxKey = v.back().first;
                }
                return maxKey;
            };
        };

        size_t countProcessedPairs = 0;
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

                    _setPhasePercentage(float(++countProcessedPairs), totalNumOfInterPairs);
                }
            }

            shuffledQueue.push_back(newVec); // Push it to our shuffled queue
        }
    }
    // else => you aren't thread-0, so go wait on barrier until thread-0 finishes shuffling
}

void ThreadContext::_setPhasePercentage(float numOfProcessed, size_t total) {
    currentJobContext_.setJobStatePercentage((numOfProcessed / total) * 100);
}

void ThreadContext::_invokeReducePhase()
{
    size_t numOfShuffleElems = JobContext::getNumOfShuffledPairs(currentJobContext_.getShuffledQueue());
    auto shuffleQueue = currentJobContext_.getShuffledQueue();

    size_t ix;
    while ((ix = currentJobContext_.atomicCounter()) < shuffleQueue.size())
    {
        currentJobContext_.invokeClientReduceRoutine(&shuffleQueue[ix],static_cast<void *>(this));

        // TODO - stage update
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

void ThreadContext::pthreadJoin()
{
    // TODO - transfer this checking to `WaitForJob`
    if (!isJoined_) {
        if (pthread_join(pthreadThread_, nullptr)) {
            systemError("[[pthread_join]] failed.");
        }
        isJoined_ = true;
    }
}