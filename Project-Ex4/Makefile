CC=g++
CXX=g++
RANLIB=ranlib

LIBH= VirtualMemory.h PhysicalMemory.h MemoryConstants.h
LIBSRC=VirtualMemory.cpp PhysicalMemory.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

INCS=-I.
CFLAGS = -Wall -Werror -Wextra -Wpedantic -std=c++11 -fpermissive  -g $(INCS)
CXXFLAGS = -Wall -Werror -Wextra -Wpedantic -std=c++11 -fpermissive  -g $(INCS)

VIRTUALMEMORYLIB = libVirtualMemory.a
TARGETS = $(VIRTUALMEMORYLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex4.tar
TARSRCS=$(LIBSRC) $(LIBH) Makefile README

all: $(TARGETS)

$(TARGETS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(TARGETS) $(VIRTUALMEMORYLIB) $(OBJ) $(LIBOBJ) *~ *core

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

