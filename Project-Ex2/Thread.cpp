//
// Created by dan-os on 30/05/2021.
//
#include "Thread.h"


/**
 * @param id Thread's ID
 */
Thread::Thread(unsigned int id) : _tid(id), _status(RUNNING), _stack(nullptr)
{
    sigsetjmp(_env, 1);
    sigemptyset(&_env->__saved_mask);
}

/**
 * @param id Thread's ID
 * @param f Thread's entry point
 */
Thread::Thread(unsigned int id, void (*f)(void)) : _tid(id), _status(READY),
                                                            _stack(new char[STACK_SIZE])
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

/**
 *
 */
Thread::~Thread()
{
    delete[] _stack;
}

/**
 *
 * @return
 */
unsigned int Thread::get_id() const
{
    return _tid;
}

/**
 *
 * @return
 */
threadStatus Thread::get_status() const
{
    return _status;
}

/**
 *
 * @param status
 */
void Thread::set_status(threadStatus status)
{
    _status = status;
}

/**
 *
 */
void Thread::set_quantum_running()
{
    ++_total_quantum;
}

/**
 *
 * @return
 */
unsigned int Thread::get_quantum_running() const
{
    return _total_quantum;
}