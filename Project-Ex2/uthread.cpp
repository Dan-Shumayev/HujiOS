//
// Created by dan-os on 31/05/2021.
//

#include "uthreads.h" // Library API
#include "uthread_utilities.h" // Masking
#include "Scheduler.h" // Scheduler, SIGVTALRM

// TODO - implement threadLibraryError messages returning EXIT_FAILURE as well

/** Assumption: this function is the first called function */
int uthread_init(int quantum_usecs)
{
   if (quantum_usecs < 1)
   {
       return threadLibraryError("Quantum length cannot be non-positive");
   }

   /** Scheduling initialization part */
   // mask timer preemption to prevent interruption of the initialization process
   SigMask timer_mask(SIGVTALRM);
   auto scheduler_init = new Scheduler(quantum_usecs); // pointer to Scheduler object
   scheduler_manager = std::unique_ptr<Scheduler>(scheduler_init); // the pointer is copied and not the
   // object itself, so no problem of copy-ctr

   return EXIT_SUCCESS;
}

int uthread_spawn(void (*f)(void))
{
    SigMask timer_mask(SIGVTALRM);

    int nextSpawnedThreadId = scheduler_manager->spawnThread(f);
    // it's assigned with the newly spawn thread's ID, otherwise with -1 (EXIT_FAILURE)
    return nextSpawnedThreadId;
}

int uthread_terminate(int tid)
{
    SigMask timer_mask(SIGVTALRM);
    return scheduler_manager->terminateThread(tid);
}

int uthread_block(int tid)
{
    SigMask timer_mask(SIGVTALRM);
    return scheduler_manager->blockThread(tid);
}

int uthread_resume(int tid)
{
    SigMask timer_mask(SIGVTALRM);
    scheduler_manager->resumeThread(tid);
}

int uthread_get_tid()
{
    // we aren't masking the timer signal, as this function doesn't affect the library
    // state (either way we may get stale value)
    return scheduler_manager->getTid();
}

int uthread_get_total_quantums()
{
    return scheduler_manager->getTotalQuantums();
}

int uthread_get_quantums(int tid)
{
    SigMask timer_mask(SIGVTALRM);

    return scheduler_manager->getThreadQuantums(tid);
}

void timerHandlerGlobal(int signo)
{
    scheduler_manager->timerHandler(signo);
}