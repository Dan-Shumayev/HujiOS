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

    int port;
    std::string cmdToRun;

    /** Creates a server */
    explicit Command(const std::string &cmdLine)
            : cmdLine(cmdLine), numOfArgs(0), port(), cmdToRun()
    {
        commandToArgs();

        bool isServer = isServerArgs();
        bool isClient = isClientArgs();

        if (!(isServer || isClient))
        {
            std::cerr << "Bad usage" << std::endl;
        }

        isServer ? turnServerOn() : toServer();
    }


private:
    /** Sends the command to the server */
    void toServer() const
    {
        Client client(port);

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
            if (numOfArgs == 2) {
                port = std::stoi(buf); // Assume the third argument is a valid port number
            }
            else if (numOfArgs >= 3) { // Fetch the actual command to be run at the server
                cmdToRun += buf + " ";
            }

            ++numOfArgs;
        }
    }
};

#endif//COMMAND_H