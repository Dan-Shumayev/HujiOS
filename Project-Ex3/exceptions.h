//
// Created by dan-os on 12/06/2021.
//

#ifndef EX3_EXCEPTIONS_H
#define EX3_EXCEPTIONS_H

const int EXIT_FAIL = -1;

/**
 * Display a system call error and exit the program
 * @param msg Detailed error information
 */
[[ noreturn ]] void systemError(const char* msg);

/**
 * Displays a thread library error and returns -1
 * @param msg Detailed error information
 * @return -1 error code
 */
int mapReduceLibraryError(const char* msg);

#endif //EX3_EXCEPTIONS_H
