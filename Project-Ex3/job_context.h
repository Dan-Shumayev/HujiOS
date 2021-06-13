//
// Created by dan-os on 09/06/2021.
//

#ifndef EX3_JOB_CONTEXT_H
#define EX3_JOB_CONTEXT_H

#include "MapReduceFramework.h"
#include "thread_context.h"
#include <pthread.h> // pthread_t
#include <vector> // std::vector
#include "Barrier.h"
#include <memory> // std::unique_ptr

class JobContext {
private:
    /** Client's API */
    const MapReduceClient client_; // client object's map and reduce routines

    /** Job-associated data */
    const InputVec inputVec_; // isn't supposed to be modified
    OutputVec outputVec_; // emit3's outputs
    JobState currJobState_;

    /** Thread-management */
    std::vector<std::unique_ptr<ThreadContext>> threadContexts_; // working threads' context
    size_t numOfThreads_; // amount of working threads
    std::atomic<size_t> lastThreadWorker_; // currently last assigned working thread
    Barrier threadsBarrier_; // used to synchronize all threads in between each phase
public:
    JobContext(const MapReduceClient& client, const InputVec& inputVec, OutputVec& outputVec, int numOfThreads)
    : client_(client),
      currJobState_({UNDEFINED_STAGE}),
      numOfThreads_(numOfThreads),
      inputVec_(inputVec),
      outputVec_(outputVec),
      threadsBarrier_(numOfThreads),
      lastThreadWorker_(0)
    {
        threadContexts_ = new ThreadContext[numOfThreads_]; // allocate numOfThread unique_ptr's
        for (size_t i = 0; i < numOfThreads_; ++i)
        {
            threadContexts_[i](i, this); // construct each of them - ThreadContext ctor
        }
    }

    std::vector<pthread_t>& getThreadWorkers() {return ;}; // TODO build a data structure for the threadWorkers

    std::vector<ThreadContext>& getThreadContexts() {return threadContexts_;};

    size_t getNumOfThreads() const {return numOfThreads_;};

    const InputVec &getInputVector() const {return inputVec_;};

    size_t getNumOfInputElems() const {return inputVec_.size();};

    void invokeClientMapRoutine(const K1* key, const V1* value, void* context) {client_.map(key, value, context);};

    void invokeReduceMapRoutine(const IntermediateVec* pairs, void* context) {client_.reduce(pairs, context);};

    size_t lastThreadAtomicGetIncrement() {return lastThreadWorker_++;};

    void barrier() {threadsBarrier_.barrier();};
};


#endif //EX3_JOB_CONTEXT_H
