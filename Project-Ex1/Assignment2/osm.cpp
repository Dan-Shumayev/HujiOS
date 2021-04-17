//
// Created by Dan Shumayev on 17/04/2021.
//
#include "osm.h" /* Our library's references */
#include <sys/time.h> /* timeval, gettimeofday */
#include <cmath> /* ceil */

#define EXIT_RETURN -1
#define LOOP_UNROLLING_FACTOR 4 /* We use that method to reduce the
 *                                  overhead of the loop-index increment */
#define LOOP_UNROLLING(operation) operation; operation; operation; operation;
#define SEC_TO_NANOSEC 1000000000
#define MICROSEC_TO_NANOSEC 1000

double osm_operation_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_RETURN;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);
    { // The number of iterations in total is #iterations as required
        unsigned int unrolled_iterations = ceil(((double) iterations / LOOP_UNROLLING_FACTOR));
        unsigned int counter = 0;
        for (unsigned int i = 0; i < unrolled_iterations; ++i) {
            LOOP_UNROLLING(++counter);
        }
    }
    gettimeofday(&end_time, nullptr);

    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
            (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta / iterations;
}

/* This function is empty and dedicated to measure the
 * CDECL procedure overhead */
void cdecl_func() {}

double osm_function_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_FAILURE;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);
    {
        for (unsigned int i = 0; i < iterations; ++i) {
            cdecl_func();
        }
    }
    gettimeofday(&end_time, nullptr);

    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
                   (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta / iterations;
}

double osm_syscall_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_FAILURE;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);
    {
        for (unsigned int i = 0; i < iterations; ++i) {
            OSM_NULLSYSCALL;
        }
    }
    gettimeofday(&end_time, nullptr);

    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
                   (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta / iterations;
}