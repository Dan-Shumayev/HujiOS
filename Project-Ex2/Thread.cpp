//
// Created by dan-os on 30/05/2021.
//
#include "thread.h"

Thread::Thread(size_t id, void (*f)(void))
    : tid_(id),
    state_(Status::READY),
    numOfQuantum_(0),
    stack_(new char[STACK_SIZE])
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
    sigemptyset(&env_->__saved_mask);
}

void Thread::set_state(const Status& state)
{
    state_ = state;
    if (state_ == Status::RUNNING)
    {
        ++numOfQuantum_;
    }
}