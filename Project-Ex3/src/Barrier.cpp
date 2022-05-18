#include "Barrier.h"
#include "exceptions.h"


Barrier::Barrier(int numThreads)
		: mutex(PTHREAD_MUTEX_INITIALIZER)
		, cv(PTHREAD_COND_INITIALIZER)
		, count(0)
		, numThreads(numThreads)
{ }


Barrier::~Barrier()
{
	if (pthread_mutex_destroy(&mutex) != 0) {
        systemError("[[Barrier]] error on pthread_mutex_destroy");
    }
	if (pthread_cond_destroy(&cv) != 0){
        systemError("[[Barrier]] error on pthread_cond_destroy");
    }
}


void Barrier::barrier()
{
	if (pthread_mutex_lock(&mutex) != 0){
        systemError("[[Barrier]] error on pthread_mutex_lock");
    }
	if (++count < numThreads) {
		if (pthread_cond_wait(&cv, &mutex) != 0){
            systemError("[[Barrier]] error on pthread_cond_wait");
        }
	} else {
		count = 0;
		if (pthread_cond_broadcast(&cv) != 0) {
			systemError("[[Barrier]] error on pthread_cond_broadcast");
        }
	}
	if (pthread_mutex_unlock(&mutex) != 0) {
        systemError("[[Barrier]] error on pthread_mutex_unlock");
    }
}
