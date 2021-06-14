//
// Created by dan-os on 12/06/2021.
//

#ifndef EX3_EXCEPTIONS_H
#define EX3_EXCEPTIONS_H

#include <iostream>
#include <cstring>


const int EXIT_FAIL = -1;

/**
 * Display a system call error and exit the program
 * Note that noreturn directive makes the program not to return from systemError invocation
 * but terminating upon its execution
 * @param msg Detailed error information
 */
[[ noreturn ]] void systemError(const std::string& msg);

/**
 * Displays a thread library error and returns -1
 * @param msg Detailed error information
 * @return -1 error code
 */
int mapReduceLibraryError(const std::string& msg);

#endif //EX3_EXCEPTIONS_H
