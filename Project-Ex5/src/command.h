#ifndef COMMAND_H
#define COMMAND_H

#include "socket.h"
#include <cstdlib>
#include <utility>
#include <vector>


/** Represents command line arguments */
struct Command {
    const std::string &cmdLine;
    int port;

    /** Creates a server */
    explicit Command(const std::string &cmdLine)
            : cmdLine(cmdLine), port(0)
    {
        // TODO
        bool isServer = isServerArgs();
        bool isClient = isClientArgs();

        if (!(isServer() || isClient()))
        {
            std::cerr << "Bad usage" << std::endl;
        }

        isServer ? turnServerOn() : toServer();
    }

    /** Sends the command to the server */
    void toServer() const
    {
        // TODO
        // writeToSocket(sock, cmdLine);
    }

    /** Turns the server on, opens a socket, listening on the given port */
    void turnServerOn() const
    {
        // TODO
    }

    /** Reads a message from a socket */
    static Command fromSocket(Socket &sock)
    {
        auto messageType = static_cast<MessageType>(readFromSocket<uint32_t>(sock));
        if (isClientArgs(messageType))
        {
            auto remoteName = readFromSocket<std::string>(sock);
            auto payload = readFromSocket<std::vector<char>>(sock);
            return Command(std::move(payload));
        }
        else if (isServerArgs(messageType))
        {
            std::vector<char> payload;
            if (messageType == MessageType::DownloadSuccess)
            {
                payload = readFromSocket<std::vector<char>>(sock);
            }
            return Command(payload);
        } else
        {
            panic("Got invalid message type");
        }
    }

    /** Checks if 'args' is a command sent by a client to be run at the server */
    static bool isClientArgs()
    {
        // TODO
        return true;
    }

    /** Checks if 'args' is a port argument upon which the server is to listen */
    static bool isServerArgs()
    {
        // TODO
        return true;
    }

private:
    void commandToArgs()
    {
        (void)cmdLine;
        (void)cmdLineArgs;
    }
};

#endif//MESSAGE_H