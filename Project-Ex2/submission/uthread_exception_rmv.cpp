//
// Created by dan-os on 02/06/2021.
//

#include "uthread_exception.h"

void uthreadSystemException(const char *msg)
{
    std::cerr << "system error: " << msg << ", errno - " << std::strerror(errno) << std::endl;
    std::exit(1);
}

int uthreadException(const char *msg)
{
    std::cerr << "thread library error: " << msg << std::endl;
    return EXIT_FAIL;
}