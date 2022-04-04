C=g++
CXX=g++
RANLIB=ranlib

LIBH=Scheduler.h thread.h uthread_utilities.h
LIBSRC=Thread.cpp Scheduler.cpp uthreads.cpp uthread_utilities.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

INCS=-I.
CFLAGS = -Wall -Wextra -std=c++11 -g $(INCS) -O2
CXXFLAGS = -Wall -Wextra -std=c++11 -g $(INCS) -O2

UTHREADLIB = libuthreads.a
TARGETS = $(UTHREADLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex2.tar
TARSRCS=$(LIBSRC) $(LIBH) Makefile README

all: $(TARGETS)

$(TARGETS): $(LIBOBJ)
		$(AR) $(ARFLAGS) $@ $^
			$(RANLIB) $@

clean:
		$(RM) $(TARGETS) $(LIBOBJ) *~ *core

depend:
		makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
		$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)
