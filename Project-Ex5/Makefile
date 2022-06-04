CXX = g++

CXXFLAGS  = -g -Wall -Wextra

TARGETS = sockets container

TAR=tar
TARSRCS = $(CONTAINER_SRCS) $(SOCKETS_SRCS) README Makefile
TARNAME = ex5.tar
TARFLAGS = -cvf

CONTAINER_SRCS = container.cpp
SOCKETS_SRCS = sockets.cpp

all: $(TARGETS)

container: $(CONTAINER_SRCS)
	$(CXX) $(CXXFLAGS) -o container container.cpp

sockets: $(SOCKETS_SRCS)
	$(CXX) $(CXXFLAGS) -o sockets sockets.cpp

tar: $(TARGETS)
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

clean:
	$(RM) $(TARGETS) $(TARNAME)