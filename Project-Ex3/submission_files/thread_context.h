//
// Created by dan-os on 12/06/2021.
//

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
    pthread_t pthreadThread_; // the actual thread worker from pthread library
    JobContext& currentJobContext_; // It's a reference (and not a smart pointer) because some of the library
    // functions receive the job handle as a void*, resulting required static_cast from
    // a smart pointer (we'd wish to define) into a void*. But, casting a smart pointer
    // defects its destruction.
    IntermediateVec intermediateVec_;
    bool isJoined_; // true iff pthread_join was called on pthreadThread

public:
    ThreadContext(size_t tid, JobContext& jobContext);

    static void *_threadEntryPoint(void *context);

    JobContext& getJobContext() {return currentJobContext_;};

    void invokeMapPhase();

    void invokeSortPhase();

    void invokeShufflePhase();

    void invokeReducePhase();

    void pthreadJoin();

    void pushIntermediateElem(IntermediatePair&& intermediatePair);

    void pushOutputElem(OutputPair&& outputPair);

    IntermediateVec& getIntermediateVec() {return intermediateVec_;};
};


#endif //EX3_THREAD_CONTEXT_H
