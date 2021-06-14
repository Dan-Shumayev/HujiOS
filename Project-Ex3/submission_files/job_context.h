//
// Created by dan-os on 09/06/2021.
//

#ifndef EX3_JOB_CONTEXT_H
#define EX3_JOB_CONTEXT_H

#include "MapReduceFramework.h" // MapReduce library API
#include "exceptions.h" // systemError, mapReduceLibraryError
#include <vector> // std::vector
#include "Barrier.h" // barrier
#include <memory> // std::unique_ptr
#include <atomic> // std::atomic
#include "thread_context.h"


class JobContext {
private:
    /** Client's API */
    const MapReduceClient& client_; // client object's map and reduce routines
    // it's a reference to `client` as it's an abstract type, and there is no rationale in copying
    // such a type, but only referring to its origin

    /** Job-associated data */
    const InputVec& inputVec_; // isn't supposed to be modified
    OutputVec outputVec_; // emit3's outputs
    JobState currJobState_;

    /** Thread-management */
    size_t numOfThreads_; // amount of working threads
    std::vector<std::unique_ptr<ThreadContext>> threadContexts_; // working threads' context
    std::atomic<size_t> lastThreadWorker_; // currently last assigned working thread
    Barrier threadsBarrier_; // used to synchronize all threads in between each phase
public:
    JobContext(const MapReduceClient& client, const InputVec& inputVec, OutputVec& outputVec, int numOfThreads);

    // TODO returning by ref - good idea?
    std::vector<std::unique_ptr<ThreadContext>>& getThreadContexts() {return threadContexts_;};

    size_t getNumOfThreads() const {return numOfThreads_;};

    const InputVec &getInputVector() const {return inputVec_;};

    size_t getNumOfInputElems() const {return inputVec_.size();};

    void invokeClientMapRoutine(const K1* key, const V1* value, void* context) {client_.map(key, value, context);};

    void invokeReduceMapRoutine(const IntermediateVec* pairs, void* context) {client_.reduce(pairs, context);};

    size_t lastThreadAtomicGetIncrement() {return lastThreadWorker_++;};

    void barrier() {threadsBarrier_.barrier();};

    void getJobDone();
};


#endif //EX3_JOB_CONTEXT_H
