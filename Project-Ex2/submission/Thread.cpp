//
// Created by dan-os on 30/05/2021.
//
#include "thread.h"
#include <signal.h>

#ifdef __x86_64__ // Pre-defined compiler macro ($ echo | gcc -E -dM -)
/* code for 64 bit Intel arch */

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

Thread::Thread()
: tid_(0), numOfQuantum_(0), stack_(nullptr), env_{} {}

Thread::Thread(int id, void (*f)(void))
    : tid_(id),
    numOfQuantum_(0),
    stack_(new char[STACK_SIZE]),
    env_{}
{
    /** Initialize the thread's execution context
     * stack base address (sp) and an entry point (f).
     * Note: each thread holds its stack on the heap section. */
    address_t sp, pc;
    sp = (address_t)stack_ + STACK_SIZE - sizeof(address_t);
    pc = (address_t)f;

    /** Define an execution context for the thread, enabling signal-handling as well.
     *  Initially, the thread isn't configured to handle any signal - that is, no
     *  signal is masked at first. */
    sigsetjmp(env_, 1);
    (env_->__jmpbuf)[JB_SP] = translate_address(sp);
    (env_->__jmpbuf)[JB_PC] = translate_address(pc);
    if (sigemptyset(&env_->__saved_mask) != 0)
    {
        uthreadSystemException("sigemptyset");
    }
}