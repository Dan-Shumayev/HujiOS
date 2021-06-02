//
// Created by dan-os on 30/05/2021.
//
#ifndef PROJECT_EX2_THREAD_H
#define PROJECT_EX2_THREAD_H

#include <memory> // smart_pointers
#include "uthreads.h" // STACK_SIZE macro
#include <setjmp.h> // sigjmp_buf
#include "uthread_exception.h" // uthreadException, uthreadSystemException

#ifdef __x86_64__ // Pre-defined compiler macro ($ echo | gcc -E -dM -)
/* code for 64 bit Intel arch */

typedef unsigned long address_t; // 64-bit
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
		"rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef size_t address_t; // 32-bit
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                : "=g" (ret)
                : "0" (addr));
    return ret;
}

#endif

/**
 * Thread object representation
 */
class Thread
{
private:
    int tid_; // Thread id in the range 0-99
    sigjmp_buf env_; // Thread's execution context
    std::unique_ptr<char[]> stack_; // Thread's stack represented by an array of STACK_SIZE bytes
    int numOfQuantum_; // Number of quantum the thread has occupied the CPU
public:
    /**
     * Default-constructor - only for the main thread (id==0).
     */
    Thread();

    /**
    * Second Thread's ctr
    * @param tid - Thread's id - should be in range 0-99
    * @param f - Thread's entry point
    */
    Thread(int tid, void (*f)(void));

    // prohibit copying Thread objects
    Thread(const Thread&) = delete;
    Thread operator=(const Thread&) = delete;

    /**
     * @return Thread's ID
     */
    int get_id() const {return tid_;}

    /**
     * @return Amount of quantum slots the thread has executed so far
     */
    int get_quantum_running() const {return numOfQuantum_;}

    /**
     * @return Amount of quantum slots the thread has executed so far
     */
    void incrementNumOfQuantum() {++numOfQuantum_;}

    /**
     * @return Returns the thread's environment struct
     */
    const sigjmp_buf& get_env() const {return env_;}
};

#endif //PROJECT_EX2_THREAD_H