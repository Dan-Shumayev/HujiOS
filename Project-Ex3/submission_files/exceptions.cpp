//
// Created by dan-os on 12/06/2021.
//

#include <iostream>
#include <cstring>
#include "exceptions.h"

void systemError(const std::string& msg)
{
    std::cerr << "system error: " << msg << ", errno - " << std::strerror(errno) << std::endl;
    exit(1);
}

int mapReduceLibraryError(const std::string& msg)
{
    std::cerr << "MapReduce library error: " << msg << std::endl;
    return EXIT_FAIL;
}