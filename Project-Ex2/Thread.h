//
// Created by dan-os on 30/05/2021.
//
#ifndef PROJECT_EX2_THREAD_H
#define PROJECT_EX2_THREAD_H

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
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

typedef unsigned int address_t;
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

/** The size of memory each threads occupies on the stack */
const int STACK_SIZE = 4096;

// TODO - should it be a class member?
/** Enumerate a thread's execution state */
enum threadStatus
{
    RUNNING,
    READY,
    BLOCKED
};

/**
 * Thread object representation
 */
class Thread
{
    unsigned int _tid; // Thread id in the range 0-99
    threadStatus _state; // Track thread's execution state
    unsigned int _total_quantum; // Amount of quantum slots this thread executed so far
    char *_stack; // Thread's stack represented by an array of STACK_SIZE bytes
    sigjmp_buf _env; // Thread's execution context TODO - should be public?
    bool _blocked; // Indicating if the thread is blocked
    // TODO - we want to differentiate between blocked_by_thread / blocked_by_mutex?

    public:
        /**
         * Thread's default ctr
         * @param tid - Thread's id - should be in range 0-99
         */
        Thread(int tid); // TODO - are we allowed to ctr a thread without an entry point?

        /**
        * Second Thread's ctr
        * @param tid - Thread's id - should be in range 0-99
        * @param f - Thread's entry point
        */
        Thread(int tid, void (*f)(void));

       // TODO - do we even have to support the 3 following methods?
       // --------------------------------------------------------- //
        /**
         * The Thread's destructor
         */
        ~Thread();

        /**
         * The Thread's cpy ctr
         * @param thread - a Thread object to copy
         */
        Thread(const Thread&  thread) = delete;

        /**
         * operator =
         * @param thread a reference to a Thread object to equal to
         * @return a Thread object
         */
        Thread operator=(const Thread& thread) = delete;
        // --------------------------------------------------------- //

        /**
         * @return Thread's ID
         */
        unsigned int get_id() const;

        /**
         * @return Current thread's execution state
         */
        threadStatus get_state() const;

        /**
         * Setting thread's execution state
         * @param state The state to set - RUNNING/READY/BLOCKED
         */
        void set_state(threadStatus state);

        /**
         * Increment thread's running time quantum by the given argument
         * @param quantum_usec The addition to total quantum running time
         */
        void set_quantum_running(unsigned int quantum_usec);

        /**
         * @return Amount of quantum slots the thread has executed so far
         */
        unsigned int get_quantum_running() const;

        /**
         * @return True iff the thread is blocked
         */
        bool is_blocked() const;

        /**
         * This method is blocking a thread
         */
        void block_thread();
};

#endif //PROJECT_EX2_THREAD_H