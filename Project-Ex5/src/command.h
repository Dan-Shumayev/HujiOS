#ifndef COMMAND_H
#define COMMAND_H

#include "socket.h"
#include "server.h"
#include "client.h"
#include <cstdlib>
#include <utility>
#include <vector>
#include <sstream>


/** Represents command line arguments */
struct Command {
    const std::string &cmdLine;

    int numOfArgs;
    std::vector<std::string> cmdArgs;

    int port;

    /** Creates a server */
    explicit Command(int argc, const std::string &cmdLine)
            : cmdLine(cmdLine), numOfArgs(argc), port()
    {
        bool isServer = isServerArgs();
        bool isClient = isClientArgs();

        if (!(isServer || isClient))
        {
            std::cerr << "Bad usage" << std::endl;
        }

        commandToArgs();
        port = std::stoi(cmdArgs[2]);

        isServer ? turnServerOn() : toServer();
    }

    /** Reads a message from a socket */
//    static Command fromSocket(Socket &sock)
//    {
//        auto messageType = static_cast<MessageType>(readFromSocket<uint32_t>(sock));
//        if (isClientArgs(messageType))
//        {
//            auto remoteName = readFromSocket<std::string>(sock);
//            auto payload = readFromSocket<std::vector<char>>(sock);
//            return Command(std::move(payload));
//        }
//        else if (isServerArgs(messageType))
//        {
//            std::vector<char> payload;
//            if (messageType == MessageType::DownloadSuccess)
//            {
//                payload = readFromSocket<std::vector<char>>(sock);
//            }
//            return Command(payload);
//        } else
//        {
//            panic("Got invalid message type");
//        }
//    }

private:
    /** Sends the command to the server */
    void toServer() const
    {
        Client client(port);

        std::string cmdToRun;
        for (auto ix = 3; ix < numOfArgs; ++ix)
        {
            cmdToRun += cmdArgs[ix] + " ";
        }

        client.runInServer(cmdToRun); // Start receiving connections
    }

    /** Turns the server on, opens a socket, listening on the given port */
    void turnServerOn() const
    {
        Server serv(port);

        serv.loop(); // Start receiving connections
    }

    /** Checks if 'args' is a command sent by a client to be run at the server */
    bool isClientArgs() const
    {
        return numOfArgs > 3; // Assume the third argument is a valid port number
    }

    /** Checks if 'args' is a port argument upon which the server is to listen */
    bool isServerArgs() const
    {
        return numOfArgs == 3; // Assume the third argument is a valid port number
    }

    void commandToArgs()
    {
        std::stringstream ss(cmdLine); // Insert the string into a stream
        std::string buf; // Have a buffer string

        while (ss >> buf) {
            cmdArgs.push_back(buf);
        }
    }
};

#endif//COMMAND_H