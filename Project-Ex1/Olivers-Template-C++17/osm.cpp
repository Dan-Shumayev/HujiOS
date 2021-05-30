//
// Created by Dan Shumayev on 17/04/2021.
//
#include "osm.h" /* Our library's references */
#include <sys/time.h> /* timeval, gettimeofday */

const static double EXIT_RETURN = -1;
const static unsigned int LOOP_UNROLLING_FACTOR = 4; /* We use that approach to reduce the
                                                      * overhead of the loop-index increment */
const static unsigned double SEC_TO_NANOSEC = 1000000000;
const static unsigned double MICROSEC_TO_NANOSEC = 1000;

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
void cdecl_func() {}

double osm_function_time(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_RETURN;
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
        return EXIT_RETURN;
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

enum class Operation {Addition, Function, Syscall};

template <Operation op>
double measure(unsigned int iterations) {
    if (iterations == 0) {
        return EXIT_RETURN;
    }
    unsigned int unrolled_iterations = (iterations + (iterations % LOOP_UNROLLING_FACTOR)) / LOOP_UNROLLING_FACTOR;
    unsigned int counters[LOOP_UNROLLING_FACTOR] = {0}; /* Avoid unrolling loop
                                                         * on the same variable as it produces
                                                         * another unwanted overhead - `Pipeline stall` */
    struct timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);
    { // The number of iterations in total is #iterations as required
        for (unsigned int i = 0; i < unrolled_iterations; ++i) {
            if constexpr (op == Operation::Addition) {
                ++counters[0];
                ++counters[1];
                ++counters[2];
                ++counters[3];
            }
            if constexpr (op == Operation::Function) {
                cdecl_func();
                cdecl_func();
                cdecl_func();
                cdecl_func();
            }
            if constexpr (op == Operation::Syscall) {
                OSM_NULLSYSCALL;
                OSM_NULLSYSCALL;
                OSM_NULLSYSCALL;
                OSM_NULLSYSCALL;
            }
        }
    }
    gettimeofday(&end_time, nullptr);
    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
                   (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta / iterations;
}