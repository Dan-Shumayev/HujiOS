//
// Created by dan-os on 02/06/2021.
//

#ifndef PROJECT_EX2_UTHREAD_EXCEPTION_H
#define PROJECT_EX2_UTHREAD_EXCEPTION_H

#include "uthread_utilities.h" // EXIT_FAILURE
#include <cstdlib> // std::exit
#include <iostream> // std::cerr
#include <cstring> // std::strerror
#include <ostream> // std::endl

/**
 * Prints to cerr a system call error and exits the program
 * @param msg Error info
 */
[[ noreturn ]] void uthreadSystemException(const char* msg);

/**
 * Prints to cerr an a thread library error and returns -1
 * @param msg Error info
 * @return -1 indicating error code
 */
int uthreadException(const char* msg);


#endif //PROJECT_EX2_UTHREAD_EXCEPTION_H
