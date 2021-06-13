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
#include <atomic> // std::atomic

class JobContext {
private:
    /** Client's API */
    const MapReduceClient& client_; // client object's map and reduce routines

    /** Job-associated data */
    const InputVec inputVec_; // isn't supposed to be modified
    OutputVec outputVec_; // emit3's outputs
    JobState currJobState_;

    /** Thread-management */
    size_t numOfThreads_; // amount of working threads
    std::vector<std::unique_ptr<ThreadContext>> threadContexts_; // working threads' context
    std::atomic<size_t> lastThreadWorker_; // currently last assigned working thread
    Barrier threadsBarrier_; // used to synchronize all threads in between each phase
public:
    JobContext(const MapReduceClient& client, const InputVec& inputVec, OutputVec& outputVec, int numOfThreads)
    : client_(client),
      inputVec_(inputVec),
      outputVec_(outputVec),
      currJobState_({UNDEFINED_STAGE}),
      numOfThreads_(numOfThreads),
      threadContexts_(numOfThreads), // TODO is it necessary?
      lastThreadWorker_(0),
      threadsBarrier_(numOfThreads_)
    {
        for (size_t i = 0; i < numOfThreads_ - 1; ++i)
        {
            threadContexts_.emplace_back(i, *this); // construct each of them - ThreadContext ctor
        }
    }

//    std::vector<pthread_t>& getThreadWorkers() {return ;}; // TODO manage a data structure for the threadWorkers
                                                                // TODO inside ThreadContext

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
