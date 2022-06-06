CXX = g++

CXXFLAGS  = -g -Wall -Wextra -Werror -std=c++11

TARGETS = sockets container

TAR=tar
TARSRCS = $(CONTAINER_SRCS) $(SOCKETS_SRCS) README Makefile
TARNAME = ex5.tar
TARFLAGS = -cvf

CONTAINER_SRCS = src/container.cpp
SOCKETS_SRCS = src/sockets.cpp

all: $(TARGETS)

container: $(CONTAINER_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

sockets: $(SOCKETS_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

tar: $(TARGETS)
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

clean:
	$(RM) $(TARGETS) $(TARNAME)
