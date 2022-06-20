#ifndef SERVER_H
#define SERVER_H

#include "utilities.h"
#include "socket.h"
#include <deque>
#include <iostream>
#include <list>
#include <string>
#include <utility>



/** Server state */
class Server
{
    Socket serverSocket;

    /** Total number of file-descriptors to be watched */
    int nfds;
    /** Set of all file descriptors communicating with the server */
    fd_set readfdSet;

    std::vector<std::unique_ptr<Socket>> clientSockets;

    /** Updates nfds and readfdSet since as per the `select()` docs:
     *  After select() has returned, readfds will be cleared of all file descriptors except for
     *  those that are ready for reading.*/
    void updateSelectParams()
    {
        nfds = std::max(serverSocket.getFd(), STDIN_FILENO) + 1;

        FD_ZERO(&readfdSet); // Initialize file-descriptor set
        FD_SET(serverSocket.getFd(), &readfdSet); // Follow new clients arriving at the server
        FD_SET(STDIN_FILENO, &readfdSet); // Watch standard input gotten from user

        // Add existing clients to our FD-set to be watched
        for (const auto& client: clientSockets)
        {
            nfds = std::max(nfds, client->getFd() + 1);
            FD_SET(client->getFd(), &readfdSet);
        }
    }

public:
    /** Creates a server
     *
     * @param port Port upon which the server listens
     */
    explicit Server(int port)
        : serverSocket(),
          nfds(std::max(STDIN_FILENO, serverSocket.getFd()) + 1),
          readfdSet(),
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

            updateSelectParams(); // Re-initialize readfdSet

            if (select(nfds, &readfdSet, nullptr,
                       nullptr, nullptr) == -1) // Block server till a new request arrives in
            {
                panic("select()");
            }

            if (FD_ISSET(STDIN_FILENO, &readfdSet)) // User input
            {
                std::string input;
                std::getline(std::cin, input);
                running = input != "quit";
            }
            if (FD_ISSET(serverSocket.getFd(), &readfdSet)) // New client communication request
            {
                acceptClient();
            }
            else // Data request from a client
            {
                handleClients();
            }
        }
    }

    /** Accepts a client (only open a dedicated socket with it) */
    void acceptClient()
    {
        std::unique_ptr<Socket> clientSocket = serverSocket.accept();

        nfds = std::max(nfds, clientSocket->getFd() + 1);
        clientSockets.push_back(std::move(clientSocket));
    }

    /** Handles all selected clients */
    void handleClients()
    {
        auto it = clientSockets.begin();

        while (it != clientSockets.end())
        {
            auto fd = (*it)->getFd();

            if (FD_ISSET(fd, &readfdSet))
            {
                handleClient(**it); // Fetch its command
                it = clientSockets.erase(it);
            } else {
                ++it; // Not selected => goto the next client
            }
        }
    }

    /** Handles a client as required
     * @param clientSocket Client socket
     */
    static void handleClient(Socket &clientSocket)
    {
        printf(CLIENT_IP_STR, clientSocket.getPeerIpAddress().c_str());

        const size_t MAX_BUF = 256; // Max command from client of size 256B
        char cmdToRun[MAX_BUF];
        readDataFromSocket(clientSocket, cmdToRun, MAX_BUF); // Get data by read()

        system(cmdToRun); // Run the provided command

        printf(SUCCESS_STR);
    }
};

#endif//SERVER_H