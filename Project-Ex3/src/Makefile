CC=g++
CXX=g++
RANLIB=ranlib

LIBH= Barrier.h job_context.h exceptions.h thread_context.h map_reduce_utils.h
LIBSRC=MapReduceFramework.cpp Barrier.cpp job_context.cpp thread_context.cpp exceptions.cpp map_reduce_utils.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

INCS=-I.
CFLAGS = -Wall -Wextra -Wpedantic -std=c++11 -g $(INCS)
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++11 -g $(INCS)

OSMLIB = libMapReduceFramework.a
TARGETS = $(OSMLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex3.tar
TARSRCS=$(LIBSRC) $(LIBH) Makefile README

all: $(TARGETS)

$(TARGETS): $(LIBOBJ)
		$(AR) $(ARFLAGS) $@ $^
			$(RANLIB) $@

clean:
		$(RM) $(TARGETS) $(OSMLIB) $(OBJ) $(LIBOBJ) *~ *core

depend:
		makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
		$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

