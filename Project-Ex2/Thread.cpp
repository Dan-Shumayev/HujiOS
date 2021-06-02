//
// Created by dan-os on 30/05/2021.
//
#include "thread.h"

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
        systemError("sigemptyset");
    }
}