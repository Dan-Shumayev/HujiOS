//
// Created by dan-os on 12/06/2021.
//

#include <iostream>
#include <cstring>
#include "exceptions.h"

void systemError(const char *msg)
{
    std::cerr << "system error: " << msg << ", errno - " << std::strerror(errno) << std::endl;
    std::exit(1);
}

int threadLibraryError(const char *msg)
{
    std::cerr << "MapReduce library error: " << msg << std::endl;
    return -1;
}