#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <cstring>
#include <string>

[[ noreturn ]] void panic(const std::string &failedFunc) {
    auto errMsg = "system error: " + failedFunc + " - ";

    std::cerr << errMsg << "with errno " << errno << ": " << std::strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}


#endif //UTILITIES_H