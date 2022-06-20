#ifndef CLIENT_H
#define CLIENT_H

#include <utility>
#include <fstream>
#include <iterator>

#include "socket.h"


/** Class responsible for client communication with server (same machine :: localhost) */
class Client
{
    int port;
    Socket sock;

public:
    /** Constructs a client sending TCP-packets to the given IP:port */
    explicit Client(int port) : port(port), sock() {}

    /** Send a command to be run at server */
    void runInServer(const std::string &command)
    {
        sock.connect(port);

        printf(CONNECTED_SUCCESSFULLY_STR);

        writeDataToSocket(sock, command.c_str(), command.length());
    }
};

#endif//CLIENT_H