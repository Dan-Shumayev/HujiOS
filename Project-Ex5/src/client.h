#ifndef CLIENT_H
#define CLIENT_H

#include <utility>
#include <fstream>
#include <iterator>

#include "command.h"
#include "socket.h"


/** Class responsible for client communication with server */
class Client
{
    const std::string& ip;
    int port;
    Socket sock;

public:
    /** Construct a client sending TCP-packets to the given IP:port */
    Client(const std::string &ip, int port) : ip(ip), port(port), sock() {}

    /** Send a command to be run at server */
    void runInServer(const std::string &command)
    {
        sock.connect(ip, port);

        printf(CONNECTED_SUCCESSFULLY_STR);

        Command cmd(command);
        cmd.toServer(sock);
    }
};

#endif//CLIENT_H