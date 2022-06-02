CXX = g++

CXXFLAGS  = -g -Wall -Wextra

TARGETS = sockets.a container.a

TAR=tar
TARSRCS = $(CONTAINER_SRCS) $(SOCKETS_SRCS) README Makefile
TARNAME = ex5.tar
TARFLAGS = -cvf

CONTAINER_SRCS = container.cpp
SOCKETS_SRCS = sockets.cpp

all: $(TARGETS)

container.a: $(CONTAINER_SRCS)
	$(CXX) $(CXXFLAGS) -o container.a container.cpp

sockets.a: $(SOCKETS_SRCS)
	$(CXX) $(CXXFLAGS) -o sockets.a sockets.cpp

tar: $(TARGETS)
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

clean:
	$(RM) $(TARGETS) $(TARNAME)