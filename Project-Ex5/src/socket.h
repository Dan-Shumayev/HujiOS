#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cassert>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <endian.h>
#include <memory>
#include <cstring>
#include "utilities.h"
#include "prints.h"


/** Wraps a socket file descriptor */
class Socket
{
    int sockFd;

    static constexpr int DEFAULT_BACKLOG = 5; // TODO: Why constexpr

    static constexpr size_t MAX_HOST_NAME_LENGTH = 255;

    /** Finds the address of the first network interface of the host */
    static in_addr getHostAddress()
    {
        char hostName[MAX_HOST_NAME_LENGTH + 1];
        if (gethostname(hostName, MAX_HOST_NAME_LENGTH) != 0)
        {
            panic("gethostname() - ");
        }

        hostent* ent = gethostbyname(hostName);
        if (ent == nullptr)
        {
            panic("gethostbyname() - ");
        }

        return *reinterpret_cast<in_addr*>(ent->h_addr_list[0]);
    }

    /** Wraps a pre-existing socket */
    explicit Socket(int fd): sockFd(fd) {}

public:
    /** Creates an unbound socket */
    Socket()
    {
        sockFd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockFd == -1)
        {
            panic("socket() - ");
        }
    }

    /** Creates an active connection on the client to a
     *  server identified by given IPv4 address and port */
    void connect(int port) const
    {
        sockaddr_in addr = {AF_INET, htons(port),
                            Socket::getHostAddress(), {}};

        if (::connect(sockFd, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)))
        {
            panic("connect() - ");
        }
    }

    /** Creates a passive connection on the host, listening for connections */
    void bindAndListen(int port) const
    {
        sockaddr_in addr = {AF_INET, htons(port),
                            Socket::getHostAddress(), {}};


        if (bind(sockFd, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)))
        {
            panic("bind() - ");
        }
        if (listen(sockFd, DEFAULT_BACKLOG))
        {
            panic("listen() - ");
        }

        printf(SERVERS_BIND_IP_STR, inet_ntoa(addr.sin_addr));
    }

    /** Blocks until a new connection arrives, returning a Socket pointing
     *  to the new client. */
    std::unique_ptr<Socket> accept() const
    {
        sockaddr addr = {};
        socklen_t addrLen = sizeof(addr);

        int clientSocketFd = ::accept(sockFd, &addr, &addrLen);
        if (clientSocketFd == -1)
        {
            panic("accept() - ");
        }

        return std::unique_ptr<Socket>(new Socket(clientSocketFd));
    }

    /** Returns IP address of peer */
    std::string getPeerIpAddress() const // TODO: see if necessary
    {
        sockaddr addr = {};
        socklen_t addrLen = sizeof(addr);

        if (getpeername(sockFd, &addr, &addrLen))
        {
            panic("getpeername() - ");
        }

        const char* stringAddr = inet_ntoa(reinterpret_cast<sockaddr_in*>(&addr)->sin_addr);
        if (stringAddr == nullptr)
        {
            panic("inet_ntoa() - ");
        }

        return stringAddr;
    }

    /** Returns the underlying socket descriptor */
    int getFd() const { return sockFd; }


    Socket(const Socket&)=delete;
    Socket& operator=(const Socket&)=delete;

    ~Socket()
    {
        if (close(sockFd))
        {
            panic("close() - ");
        }
    }
};


/* Overloaded wrappers for converting primitive types between
 * network and host variants */
// TODO: inspect

char toNetwork(char ch)
{
    return ch;
}

uint32_t toNetwork(uint32_t i)
{
    return htobe32(i);
}

uint64_t toNetwork(uint64_t i)
{
    return htobe64(i);
}

char fromNetwork(char ch)
{
    return ch;
}

uint32_t fromNetwork(uint32_t i)
{
    return be32toh(i);
}

uint64_t fromNetwork(uint64_t i)
{
    return be64toh(i);
}


/** Write 'len' bytes originating from 'data' into socket */
void writeBytesToSocket(Socket& sock, const char* data, size_t len)
{
    while (len > 0)
    {
        ssize_t res = write(sock.getFd(), data, len);
        if (res == -1)
        {
            panic("write failure");
        }
        if (res == 0)
        {
            panic("Other side has disconnected too early");
        }
        len -= res;
        data += res;
    }
}

/** Reads 'len' bytes originating from socket into 'data' */
void readBytesFromSocket(Socket& sock, char* data, size_t len)
{
    while (len > 0)
    {
        ssize_t res = read(sock.getFd(), data, len);
        if (res == -1)
        {
            panic("read() - ");
        }
        if (res == 0)
        {
            panic("Other side has disconnected too early");
        }
        len -= res;
        data += res;
    }
}

/*  The following are wrappers for reading/writing various C++ types into a socket */
// TODO: inspect

/** Reads a primitive type from socket */
template<typename T>
T readFromSocket(Socket& sock)
{
    T dataNetwork{};
    readBytesFromSocket(sock, reinterpret_cast<char*>(&dataNetwork), sizeof(T));
    return fromNetwork(dataNetwork);
}

/** Reads a vector of bytes from socket */
template<>
std::vector<char> readFromSocket(Socket& sock)
{
    auto size = readFromSocket<size_t>(sock);
    std::vector<char> vec(size);
    readBytesFromSocket(sock, vec.data(), size);
    return vec;
}

/** Reads a string from socket */
template<>
std::string readFromSocket(Socket& sock)
{
    auto size = readFromSocket<size_t>(sock);
    std::string string(size, ' ');
    // beginning with C++17, std::string::data returns a non-const char*,
    // for now a const cast is needed.
    readBytesFromSocket(sock, const_cast<char*>(string.data()), size);
    return string;
}

#endif //SOCKET_H