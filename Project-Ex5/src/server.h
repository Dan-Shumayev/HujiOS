#ifndef SERVER_H
#define SERVER_H

#include "utilities.h"
#include "command.h"
#include "socket.h"
#include <deque>
#include <iostream>
#include <list>
#include <string>
#include <utility>


// TODO: handleClient()

/** Server state */
class Server
{
    Socket serverSocket;

    /** Total number of file-descriptors to be watched */
    int nfds;
    /** Set of all file descriptors communicating with the server */
    fd_set readfsSet;

    std::list<std::unique_ptr<Socket>> clientSockets;

    /** Updates nfds and readfsSet since as per the `select()` docs:
     *  After select() has returned, readfds will be cleared of all file descriptors except for
     *  those that are ready for reading.*/
    void updateSelectParameters()
    {
        nfds = std::max(serverSocket.getFd(), STDIN_FILENO) + 1;

        FD_ZERO(&readfsSet); // Initialize file-descriptor set
        FD_SET(serverSocket.getFd(), &readfsSet); // Follow new clients arriving at the server
        FD_SET(STDIN_FILENO, &readfsSet); // Watch standard input gotten from user

        // Add existing clients to our FD-set to be watched
        for (const auto& client: clientSockets)
        {
            nfds = std::max(nfds, client->getFd() + 1);
            FD_SET(client->getFd(), &readfsSet);
        }
    }

public:
    /** Creates server
     *
     * @param port Listening socket port
     */
    explicit Server(int port)
        : serverSocket(),
          nfds(std::max(STDIN_FILENO, serverSocket.getFd()) + 1),
          readfsSet(),
          clientSockets()
    {
        serverSocket.bindAndListen(port);
    }

    /** Server loops, accepts client connections until receiving "quit" */
    void loop()
    {
        bool running = true;

        while (running)
        {
            printf(WAIT_FOR_CLIENT_STR);

            updateSelectParameters(); // Re-initialize readfsSet

            if (select(nfds, &readfsSet, nullptr,
                       nullptr, nullptr) == -1) // Block server till a new request arrives in
            {
                panic("select() - ");
            }

            if (FD_ISSET(serverSocket.getFd(), &readfsSet)) // New client communication request
            {
                acceptClient();
            }

            if (FD_ISSET(STDIN_FILENO, &readfsSet)) // User input
            {
                std::string input;
                std::getline(std::cin, input);
                running = input != "quit";
            }
            else // Data request from a client
            {
                handleClients();
            }
        }
    }

    /** Accepts a client (without handling it) */
    void acceptClient()
    {
        std::unique_ptr<Socket> clientSocket = serverSocket.accept();

        nfds = std::max(nfds, clientSocket->getFd() + 1);
        clientSockets.push_back(std::move(clientSocket));
    }

    /** Handles all clients */
    void handleClients()
    {
        auto it = clientSockets.begin();

        while (it != clientSockets.end())
        {
            auto fd = (*it)->getFd();

            if (FD_ISSET(fd, &readfsSet))
            {
                handleClient(**it);
                it = clientSockets.erase(it);
            } else {
                ++it;
            }
        }
    }

    /** Handles a client
     * @param clientSocket Client socket
     */
    static void handleClient(Socket &clientSocket)
    {
        printf(CLIENT_IP_STR, clientSocket.getPeerIpAddress().c_str());

        auto msg = Command::fromSocket(clientSocket);
//        if (msg == Args)
//        {
//            auto bytes = loadFile(fullPath);
//            auto res = Message(MessageType::DownloadSuccess, bytes);
//            res.toSocket(clientSocket);
//        }

        printf(SUCCESS_STR);
    }
};

#endif//SERVER_H