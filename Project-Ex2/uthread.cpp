//
// Created by dan-os on 31/05/2021.
//

#include "uthreads.h" // Library API
#include "uthread_utilities.h" // Masking
#include "Scheduler.h" // Scheduler, SIGVTALRM

using threadEntryPoint = void(*)(void);

const int EXIT_FAILURE = -1;
const int EXIT_SUCCESS = 0;

/** Has to be global as any function may use the scheduler. Smart pointer as a wrapper */
std::unique_ptr<Scheduler> scheduler_manager = nullptr;

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
   auto scheduler_init = new Scheduler(quantum_usecs);
   scheduler_manager = std::unique_ptr<Scheduler>(scheduler_init); //TODO Black box meanwhile

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
    return scheduler_manager->blockThread(tid); // TODO black box meanwhile
}

int uthread_resume(int tid)
{
    SigMask timer_mask(SIGVTALRM);
    scheduler_manager->resumeThread(tid); // TODO black box meanwhile
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