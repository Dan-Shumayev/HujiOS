CXX = g++

CXXFLAGS  = -g -Wall -Wextra -Werror -std=c++11

TARGETS = sockets container

TAR=tar
TARSRCS = $(CONTAINER_SRCS) $(SOCKETS_SRCS) README Makefile
TARNAME = ex5.tar
TARFLAGS = -cvf

CONTAINER_SRCS = src/container.cpp src/utilities.h
SOCKETS_SRCS = src/sockets.cpp src/utilities.h src/prints.h src/server.h src/client.h src/command.h

all: $(TARGETS)

container: $(CONTAINER_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

sockets: $(SOCKETS_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

tar: $(TARGETS)
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

clean:
	$(RM) $(TARGETS) $(TARNAME)
