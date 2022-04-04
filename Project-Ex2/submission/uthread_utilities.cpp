//
// Created by dan-os on 01/06/2021.
//

#include "uthread_utilities.h"

void uthreadSystemException(const char *msg)
{
    std::cerr << "system error: " << msg << ", errno - " << std::strerror(errno) << std::endl;
    std::exit(1);
}

int uthreadException(const char *msg)
{
    std::cerr << "thread library error: " << msg << std::endl;
    return EXIT_FAIL;
}

SigMask::SigMask(int signo)
: sigset_{}
{
    if (sigaddset(&sigset_, signo))
    {
        uthreadSystemException("sigaddset failed at sig-mask ctor");
    }
    if (sigprocmask(SIG_BLOCK, &sigset_, nullptr))
    {
        uthreadSystemException("sigprocmask block failed at sig-mask");
    }
}
SigMask::~SigMask()
{
    if (sigprocmask(SIG_UNBLOCK, &sigset_, nullptr))
    {
        uthreadSystemException("sigprocmask unblock failed at sig-mask");
    }
}