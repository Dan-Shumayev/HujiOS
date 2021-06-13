//
// Created by dan-os on 12/06/2021.
//

#ifndef EX3_THREAD_CONTEXT_H
#define EX3_THREAD_CONTEXT_H


#include <cstddef> // size_t
#include "job_context.h" // JobContext

class ThreadContext {
private:
    const size_t pthread_thread_id_;
    JobContext& currentJobContext_; // It's a reference (and not a smart pointer) because some of the library
    // functions receive the job handle as a void*, resulting required static_cast from
    // a smart pointer (we'd wish to define) into a void*. But, casting a smart pointer
    // defects its destruction.
    IntermediateVec intermediateVec_; // TODO - associated logic

public:
    ThreadContext(size_t tid, JobContext& jobContext)
    : pthread_thread_id_(tid), currentJobContext_(jobContext) {}

    JobContext& getJobContext() {return currentJobContext_;};

    size_t getThreadId() const {return pthread_thread_id_;}; // TODO - Manage pthread data inside ThreadContext

    void invokeMapPhase();

    void invokeSortPhase();

    void invokeShufflePhase();

    void invokeReducePhase();
};


#endif //EX3_THREAD_CONTEXT_H
