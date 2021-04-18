#include "osm.h"
#include <sys/time.h>

/** 1 sec in ns */
const double SEC_TO_NANO = 1000000000.0;

/** 1 us in ns */
const double MICROSEC_TO_NANO = 1000.0;

/** Unrolling factor used for addition instructions */
const unsigned int ADD_UNROLLING_FACTOR = 4;

/***
 * Generic function for timing an operation
 * @tparam Function Callable
 * @param iterations Number of iterations to repeat operation
 * @param operation Operation to time, may or may not be inlined depending on what's being measuring.
 * @param unrollingFactor Unrolling factor within 'operation', needed to calculate the right average
 * @return Average time for performing operation, in nanoseconds, or -1 in case of error.
 */
template<class Function>
static double inline osm_time(unsigned int iterations, Function operation, int unrollingFactor = 1)
{
    timeval startTime = {}, endTime = {};
    if (iterations == 0)
    {
        return -1;
    }
    if (gettimeofday(&startTime, nullptr))
    {
        return -1;
    }
    for (unsigned int i=0; i < iterations; ++i)
    {
        operation();
    }
    if (gettimeofday(&endTime, nullptr))
    {
        return -1;
    }
    return ((double)(endTime.tv_sec - startTime.tv_sec) * SEC_TO_NANO +
           (double)(endTime.tv_usec - startTime.tv_usec) * MICROSEC_TO_NANO)/(double)(unrollingFactor * iterations);
}

/**
 * Performs 4 addition operations
 */
static inline __attribute__((always_inline)) void unrolledAdd()
{
    // use ASM to ensure this won't be optimized away, as can be done with plain integer addition
    // and unlike adding volatile integers, this doesn't have the overhead of 'mov'
    // moreover, use different registers for pipelining
    asm ("add %%eax, %%eax\n"
         "add %%ebx, %%ebx\n"
         "add %%ecx, %%ecx\n"
         "add %%edx, %%edx"
    :
    :
    : "eax", "ebx", "ecx", "edx"
    );
}


/** A function call that will never be optimized away
 */
static void __attribute__ ((noinline)) empty_function_call()
{
    asm("");
}

/** Calls a no-op syscall.
 *  Inline to avoid overhead of function call itself(even though its negligible) */
static inline __attribute__((always_inline)) void perform_syscall()
{
    OSM_NULLSYSCALL;
}

/** Functions from header */


double osm_operation_time(unsigned int iterations)
{
    return osm_time(iterations, unrolledAdd, ADD_UNROLLING_FACTOR);
}


double osm_function_time(unsigned int iterations)
{
    return osm_time(iterations, empty_function_call);
}


double osm_syscall_time(unsigned int iterations)
{
    return osm_time(iterations, perform_syscall);
}
