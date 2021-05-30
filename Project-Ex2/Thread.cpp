//
// Created by dan-os on 30/05/2021.
//
#include "Thread.h"

// TODO - how to determine the initial state of a thread - RUNNING/READY?
// should be READY, see the state diagram
// TODO - _total_quantum of a thread should be initialized with 0 or 1?
// 0, it is inreased when it changes to RUNNING

Thread::Thread(unsigned int id) : _tid(id), _state(RUNNING), _stack(nullptr)
{
    sigsetjmp(_env, 1);
    sigemptyset(&_env->__saved_mask);
}

Thread::Thread(unsigned int id, void (*f)(void))
    : _tid(id)
    , _state(READY)
    , _total_quantum(0)
    , _stack(new char[STACK_SIZE])
{
    /** Initialize the thread's execution context
     * stack base address (sp) and an entry point (f).
     * Note: each thread holds its stack on the heap section. */
    address_t sp, pc;
    sp = (address_t)_stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t)f;

    /** Define an execution context for the thread, enabling signal-handling as well.
     *  Initially, the thread isn't configured to handle any signal - that is, no
     *  signal is masked at first. */
    sigsetjmp(_env, 1);
    (_env->__jmpbuf)[JB_SP] = translate_address(sp);
    (_env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&_env->__saved_mask);
}

Thread::~Thread()
{
    delete[] _stack;
}

unsigned int Thread::get_id() const
{
    return _tid;
}

threadStatus Thread::get_state() const
{
    return _state;
}

void Thread::set_state(threadStatus state)
{
    _state = state;
}

void Thread::set_quantum_running()
{
    ++_total_quantum; // this should happen in set_state, if state transitions to RUNNING
}

unsigned int Thread::get_quantum_running() const
{
    return _total_quantum;
}