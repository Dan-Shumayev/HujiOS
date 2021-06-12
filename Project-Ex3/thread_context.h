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
    JobContext* currentJobContext_;

public:
    ThreadContext(size_t tid, JobContext* jobContext)
    : pthread_thread_id_(tid), currentJobContext_(jobContext) {}

    JobContext& getJobContext() const {return *currentJobContext_;};

    size_t getThreadId() const {return pthread_thread_id_;};
};


#endif //EX3_THREAD_CONTEXT_H
