//
// Created by Dan Shumayev on 17/04/2021.
//
#include "osm.h" /* Our library's references */
#include <sys/time.h> /* timeval, gettimeofday */
#include <cstdlib> /* EXIT_FAILURE macro, exit */

#define SEC_TO_NANOSEC 1000000000
#define MICROSEC_TO_NANOSEC 1000

double osm_operation_time(unsigned int iterations) {
    if (iterations == 0) {
        return exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);
    {
        unsigned int counter = 0;
        for (unsigned int i = 0; i < iterations; ++i) {
            ++counter;
        }
    }
    gettimeofday(&end_time, nullptr)

    double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
            (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
    return delta;
}