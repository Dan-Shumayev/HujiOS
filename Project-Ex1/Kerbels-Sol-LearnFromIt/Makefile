CC=g++
CXX=g++
RANLIB=ranlib

LIBSRC=osm.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

RUNNERSRC=runner.cpp
RUNNEROBJ=$(RUNNERSRC:.cpp=.o)

INCS=-I.
CFLAGS = -Wall -std=c++11 -g -O2 $(INCS)
CXXFLAGS = -Wall -std=c++11 -g -O2 $(INCS)

OSMLIB = libosm.a
EXE=runner
TARGETS = $(OSMLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex1.tar
TARSRCS=$(LIBSRC) Makefile README plot.png

all: $(TARGETS)


$(TARGETS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

$(EXE): $(OSMLIB) $(RUNNEROBJ)

clean:
	$(RM) $(TARGETS) $(OSMLIB) $(OBJ) $(LIBOBJ) $(RUNNEROBJ) *~ *core

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC) $(RUNNERSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)
