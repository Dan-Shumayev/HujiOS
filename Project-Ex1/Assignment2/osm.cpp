//
// Created by Dan Shumayev on 17/04/2021.
//
#include "osm.h" /* Our library's references */
#include <sys/time.h> /* timeval, gettimeofday */

const static double EXIT_RETURN = -1;
const static unsigned int LOOP_UNROLLING_FACTOR = 4; /* We use that approach to reduce the
                                                      * overhead of the loop-index increment */
const static double SEC_TO_NANOSEC = 1000000000.0;
const static double MICROSEC_TO_NANOSEC = 1000.0;

double osm_operation_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_RETURN;
    }

    unsigned int unrolled_iterations = (iterations + (iterations % LOOP_UNROLLING_FACTOR)) / LOOP_UNROLLING_FACTOR;
    struct timeval start_time, end_time;
    if (gettimeofday(&start_time, nullptr)) {
        return EXIT_RETURN;
    }
    { // The number of iterations in total is #iterations as required
        unsigned int counter1, counter2, counter3, counter4 = 0; /* Avoid unrolling loop
                                                                  * on the same variable as it produces
                                                                  * another unwanted overhead - `Pipeline stall` */
        for (unsigned int i = 0; i < unrolled_iterations; ++i) {
            ++counter1;
            ++counter2;
            ++counter3;
            ++counter4;
        }
    }
    if (gettimeofday(&end_time, nullptr)) {
        return EXIT_RETURN;
    }

    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
            (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta / iterations;
}

/* This function is empty and dedicated to measure the
 * CDECL procedure overhead */
static void __attribute__ ((noinline)) cdecl_func() {}

double osm_function_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_RETURN;
    }

    unsigned int unrolled_iterations = (iterations + (iterations % LOOP_UNROLLING_FACTOR)) / LOOP_UNROLLING_FACTOR;
    struct timeval start_time, end_time;
    if (gettimeofday(&start_time, nullptr)) {
        return EXIT_RETURN;
    }
    {
        for (unsigned int i = 0; i < unrolled_iterations; ++i) {
            cdecl_func();
            cdecl_func();
            cdecl_func();
            cdecl_func();
        }
    }
    if (gettimeofday(&end_time, nullptr)) {
        return EXIT_RETURN;
    }

    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
                   (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta / iterations;
}

double osm_syscall_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_RETURN;
    }

    unsigned int unrolled_iterations = (iterations + (iterations % LOOP_UNROLLING_FACTOR)) / LOOP_UNROLLING_FACTOR;
    struct timeval start_time, end_time;
    if (gettimeofday(&start_time, nullptr)) {
        return EXIT_RETURN;
    }
    {
        for (unsigned int i = 0; i < unrolled_iterations; ++i) {
            OSM_NULLSYSCALL;
            OSM_NULLSYSCALL;
            OSM_NULLSYSCALL;
            OSM_NULLSYSCALL;
        }
    }
    if (gettimeofday(&end_time, nullptr)) {
        return EXIT_RETURN;
    }

    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
                   (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta / iterations;
}