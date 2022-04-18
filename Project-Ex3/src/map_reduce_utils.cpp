#include "map_reduce_utils.h"
#include "exceptions.h"

ApplyMutex::ApplyMutex(pthread_mutex_t &jobMutex): m_jobMutex(jobMutex)
{
    if (pthread_mutex_lock(&jobMutex))
    {
        systemError("[[pthread_mutex_lock]] failed.");
    }
}

ApplyMutex::~ApplyMutex()
{
    if (pthread_mutex_unlock(&m_jobMutex))
    {
        systemError("[[pthread_mutex_unlock]] failed.");
    }
}