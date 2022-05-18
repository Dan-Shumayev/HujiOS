#include "exceptions.h"

void systemError(const std::string& msg)
{
    std::cerr << "system error: " << msg << ", errno - " << std::strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}

int mapReduceLibraryError(const std::string& msg)
{
    std::cerr << "MapReduce library error: " << msg << std::endl;
    return EXIT_FAIL;
}