//
// Created by Dan Shumayev on 17/04/2021.
//
#include "osm.h" /* Our library's references */
#include <sys/time.h> /* timeval, gettimeofday */
#include <cmath> /* ceil */

const static double EXIT_RETURN = -1;
const static unsigned int LOOP_UNROLLING_FACTOR = 4; /* We use that method to reduce the
                                                      * overhead of the loop-index increment */
const static unsigned int SEC_TO_NANOSEC = 1000000000;
const static unsigned int MICROSEC_TO_NANOSEC = 1000;

double osm_operation_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_RETURN;
    }

    unsigned int unrolled_iterations = (iterations + (iterations % LOOP_UNROLLING_FACTOR)) / LOOP_UNROLLING_FACTOR;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);
    { // The number of iterations in total is #iterations as required
        unsigned int counter = 0;
        for (unsigned int i = 0; i < unrolled_iterations; ++i) {
            ++counter;
            ++counter;
            ++counter;
            ++counter;
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