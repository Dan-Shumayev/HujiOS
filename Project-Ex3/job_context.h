//
// Created by dan-os on 09/06/2021.
//

#ifndef EX3_JOB_CONTEXT_H
#define EX3_JOB_CONTEXT_H

#include "MapReduceFramework.h"
#include "thread_context.h"
#include <pthread.h> // pthread_t
#include <vector> // std::vector

class JobContext {
private:
    const MapReduceClient& client_; // client object's map and reduce routines
    const InputVec& inputVec_; // isn't supposed to be modified
    OutputVec& outputVec_; // emit3's outputs
    JobState currJobState_;

    /** Thread-management */
    std::vector<pthread_t> threadWorkers_; // the actual threads in charge of doing the job
    std::vector<ThreadContext> threadContexts_; // working threads' context on the job
    std::atomic<size_t> lastThreadWorker_; // the last assigned working thread
    std::unique_ptr<Barrier> threadsBarrier_; // it's used to synchronize all threads in between each phase
public:
    JobContext(const MapReduceClient& client, const InputVec& inputVec, OutputVec& outputVec, int numOfThreads)
    : client_(client),
      currJobState_({UNDEFINED_STAGE}),
      inputVec_(inputVec),
      outputVec_(outputVec),
      threadWorkers_(numOfThreads),
      threadsBarrier_(numOfThreads),
      lastThreadWorker_(0)
    {
        for (size_t i = 0; i < numOfThreads; ++i)
        {
            threadContexts_[i] = new ThreadContext(i, &this);
        }
    }

    std::vector<pthread_t>& getThreadWorkers() {return threadWorkers_;};

    std::vector<ThreadContext>& getThreadContexts() {return threadContexts_;};

    size_t getNumOfThreads() {return threadWorkers_.size();};

    const InputVec &getInputVector() const {return inputVec_;};

    size_t getNumOfInputElems() const {return inputVec_.size();};

    void invokeClientMapRoutine(const K1* key, const V1* value, void* context) {client_.map(key, value, context);};

    std::atomic<size_t> lastThreadAtomicGetIncrement() {return lastThreadWorker_++;};

    void barrier() {threadsBarrier_->barrier;};
};


#endif //EX3_JOB_CONTEXT_H
