#ifndef EX3_THREAD_CONTEXT_H
#define EX3_THREAD_CONTEXT_H


#include <cstddef> // size_t
#include <pthread.h> // pthread_t
#include <algorithm> // std::sort
#include <map>
#include "exceptions.h"
#include "MapReduceClient.h"

// TODO - use those to implement Shuffle
struct K2PointerComp {
    bool operator()(const K2* first, const K2* second) const
    {
        return (*first < *second);
    }
};

typedef std::map<K2*, std::vector<V2 *>, K2PointerComp> IntermediateMap;

/** forward-declaration to break the include-cycle */
class JobContext;

class ThreadContext {
private:
    const size_t thread_id_;

    /** the actual thread worker from pthread library */
    pthread_t pthreadThread_;

    JobContext& currentJobContext_;
    // It's a reference (and not a smart pointer) because some of the library
    // functions receive the job handle as a void*, resulting required static_cast from
    // a smart pointer (we'd wish to define) into a void*. But, casting a smart pointer
    // defects its destruction.
    IntermediateVec intermediateVec_;

    /** true iff pthread_join was called on pthreadThread */
    bool isJoined_;

public:
    ThreadContext(size_t tid, JobContext& jobContext);

    static void *_threadEntryPoint(void *context); // TODO - consider implementing separated entry point for thread-0
                                                    // TODO ... as he exclusively invokes ShufflePhase

    JobContext& getJobContext() {return currentJobContext_;};

    void invokeMapPhase();

    void invokeSortPhase();

    void invokeShufflePhase();

    void invokeReducePhase();

    void pthreadJoin();

    void pushIntermediateElem(IntermediatePair&& intermediatePair);

    void pushOutputElem(OutputPair&& outputPair);

    IntermediateVec& getIntermediateVec() {return intermediateVec_;};

    void updateCurrentPercentage(size_t numOfInputElems);

    void _setPhasePercentage(float numOfProcessed, size_t total);

    void updateCurrentPercentageReduce(size_t currProcessed, size_t numOfInputElems);
};


#endif //EX3_THREAD_CONTEXT_H
