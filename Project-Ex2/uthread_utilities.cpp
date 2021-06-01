//
// Created by dan-os on 01/06/2021.
//

#include "uthread_utilities.h"

// TODO - implement systemError

SigMask::SigMask(int signo)
: sigset_{}
{
    if (sigaddset(&sigset_, signo))
    {
        systemError("sigaddset at sig-mask ctor");
    }
    if (sigprocmask(SIG_BLOCK, &sigset_, nullptr))
    {
        systemError("sigprocmask block at sig-mask");
    }
}
SigMask::~SigMask()
{
    if (sigprocmask(SIG_UNBLOCK, &sigset_, nullptr))
    {
        systemError("sigprocmask unblock at sig-mask");
    }
}