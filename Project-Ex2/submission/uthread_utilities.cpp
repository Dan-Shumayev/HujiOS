//
// Created by dan-os on 01/06/2021.
//

#include "uthread_exception.h"
#include "uthread_utilities.h"

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