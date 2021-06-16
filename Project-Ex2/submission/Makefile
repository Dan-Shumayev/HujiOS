C=g++
CXX=g++
RANLIB=ranlib

LIBH=*.h
LIBSRC=*.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

INCS=-I.
CFLAGS = -Wall -Wextra -std=c++11 $(INCS)
CXXFLAGS = -Wall -Wextra -std=c++11 $(INCS)

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
