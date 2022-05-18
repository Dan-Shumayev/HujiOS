#ifndef EX3_MAP_REDUCE_UTILS_H
#define EX3_MAP_REDUCE_UTILS_H

#include <pthread.h>

class ApplyMutex {
public:
    explicit ApplyMutex(pthread_mutex_t &jobMutex);

    ApplyMutex() = delete;

    ~ApplyMutex();

private:
    pthread_mutex_t &m_jobMutex; // mutex for job purposes
};


#endif //EX3_MAP_REDUCE_UTILS_H