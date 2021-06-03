C=g++
CXX=g++
RANLIB=ranlib

LIBH = thread.h Scheduler.h uthread_exception.h uthread_utilities.h
LIBSRC = uthreads.cpp Scheduler.cpp Thread.cpp uthread_exception.cpp uthread_utilities.cpp
$(LIBOBJ): $(LIBH) $(LIBSRC)
	    $(CC) $(CFLAGS) -c $(LIBSRC)

INCS=-I.
CFLAGS = -Wall -std=c++11 -g $(INCS)
CXXFLAGS = -Wall -std=c++11 -g $(INCS)

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
		$(RM) $(TARGETS) $(UTHREADLIB) $(OBJ) $(LIBOBJ) *~ *core

depend:
		makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
		$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

