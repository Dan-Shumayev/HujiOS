#ifndef EX3_THREAD_CONTEXT_H
#define EX3_THREAD_CONTEXT_H


#include <cstddef> // size_t
#include <pthread.h> // pthread_t
#include <algorithm> // std::sort
#include "exceptions.h"
#include "MapReduceClient.h"


/** forward-declaration to break the include-cycle */
class JobContext;

class ThreadContext {
private:
    const size_t thread_id_;

    /** the actual thread worker from pthread library */
    pthread_t pthreadThread_;

    JobContext& currentJobContext_;

    /** It's a reference (and not a smart pointer) because some of the library
        functions receive the job handle as a void*, resulting required static_cast from
        a smart pointer (we'd wish to define) into a void*. But, casting a smart pointer
        defects its destruction. */
    IntermediateVec intermediateVec_;

    /** true iff pthread_join was called on pthreadThread */
    bool isJoined_;

public:
    ThreadContext(size_t tid, JobContext& jobContext);

    /** Internal methods */

    static void *_threadEntryPoint(void *context);

    JobContext& _getJobContext() {return currentJobContext_;};

    void _invokeMapPhase();

    void _invokeSortPhase();

    void _invokeShufflePhase();

    void _invokeReducePhase();

    void _setPhasePercentage(float numOfProcessed, size_t total);

    /** External methods */

    void pthreadJoin();

    void pushIntermediateElem(IntermediatePair&& intermediatePair);

    void pushOutputElem(OutputPair&& outputPair);

    IntermediateVec& getIntermediateVec() {return intermediateVec_;};
};


#endif //EX3_THREAD_CONTEXT_H
