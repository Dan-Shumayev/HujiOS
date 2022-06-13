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


// TODO: inspect - nfds update; STDIN_FILENO; handleClient(s)()

/** Server state */
class Server
{
    Socket servingSocket;

    int nfds;
    fd_set readSet;
    std::list<std::unique_ptr<Socket>> clientSockets;

    /** Updates nfds and readSet */
    void updateSelectParameters()
    {
        nfds = std::max(servingSocket.getFd(), STDIN_FILENO) + 1;

        FD_ZERO(&readSet);
        FD_SET(servingSocket.getFd(), &readSet);
        FD_SET(STDIN_FILENO, &readSet);

        for (const auto& client: clientSockets)
        {
            nfds = std::max(nfds, client->getFd() + 1);
            FD_SET(client->getFd(), &readSet);
        }
    }

public:
    /** Creates server
     *
     * @param port Listening socket port
     */
    explicit Server(int port)
        : servingSocket(),
          nfds(std::max(STDIN_FILENO, servingSocket.getFd()) + 1),
          readSet(),
          clientSockets()
    {
        servingSocket.bindAndListen(port);
    }

    /** Server loops, accepts client connections until receiving "quit" */
    void loop()
    {
        bool running = true;

        while (running)
        {
            printf(WAIT_FOR_CLIENT_STR);

            updateSelectParameters();
            if (select(nfds, &readSet, nullptr,
                       nullptr, nullptr) == -1)
            {
                panic("select() - ");
            }

            if (FD_ISSET(servingSocket.getFd(), &readSet))
            {
                acceptClient();
            }
            if (FD_ISSET(STDIN_FILENO, &readSet))
            {
                std::string input;
                std::getline(std::cin, input);
                running = input != "quit";
            }
            else
            {
                handleClients();
            }
        }
    }

    /** Accepts a client (without handling it) */
    void acceptClient()
    {
        std::unique_ptr<Socket> clientSocket = servingSocket.accept();

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

            if (FD_ISSET(fd, &readSet))
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
        if (!isRequestMessage(msg.messageType))
        {
            panic("Expected request from client, got something else");
        }
        std::string fullPath = toFullPath(msg.remoteName);
        printf(CLIENT_COMMAND_STR, msg.messageType == MessageType::Upload ? 'u': 'd');
        printf(FILENAME_STR, msg.remoteName.c_str());
        printf(FILE_PATH_STR, fullPath.c_str());
        if (!isFilenameValid(msg.remoteName))
        {
            panic("Filename passed by client is not valid");
            auto res = Message(MessageType::FilenameError);
            res.toSocket(clientSocket);
            return;
        }
        try
        {
            if (msg.messageType == MessageType::Upload)
            {
                saveFile(msg.payload, fullPath);
                auto res = Message(MessageType::UploadSuccess);
                res.toSocket(clientSocket);
            }
            else if (msg.messageType == MessageType::Download)
            {
                auto bytes = loadFile(fullPath);
                auto res = Message(MessageType::DownloadSuccess, bytes);
                res.toSocket(clientSocket);
            }
            else
            {
                panic("Unsupported/invalid request message");
            }
            printf(SUCCESS_STR);
        }
        catch (const std::ios::failure &)
        {
            panic("IO exception while handling client request");
            auto res = Message(MessageType::FileError);
            res.toSocket(clientSocket);
        }
    }
};

#endif//SERVER_H