#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "metro.h"
#include "helpers.h"
#include "http/http_parser.h"
#include "http/http_writer.h"

constexpr int BACKLOG_SIZE = 128;
constexpr int BUFFER_SIZE  = 8192;

class Server {
    Metro& metro;
    int port;

public:
    Server(Metro& metroInstance, int listenPort)
        : metro(metroInstance), port(listenPort) {}

    void listen() {
        int serverSocket = createSocket();
        bindSocket(serverSocket);
        startListen(serverSocket);

        std::cout << "Listening on port " << port << "\n";

        while (true) {
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket < 0) continue;

            handleConnection(clientSocket);
            close(clientSocket);
        }
    }

private:
    int createSocket() {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        int reuseAddress = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   &reuseAddress, sizeof(reuseAddress));
        return serverSocket;
    }

    void bindSocket(int serverSocket) {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(serverSocket,
                 reinterpret_cast<sockaddr*>(&address),
                 sizeof(address)) < 0) {
            perror("bind");
            exit(1);
        }
    }

    void startListen(int serverSocket) {
        if (::listen(serverSocket, BACKLOG_SIZE) < 0) {
            perror("listen");
            exit(1);
        }
    }

    void setTimeout(int socketFd) {
        timeval timeout{};
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout));
        setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO,
                   &timeout, sizeof(timeout));
    }

    void handleConnection(int clientSocket) {
        setTimeout(clientSocket);

        bool keepAlive = true;

        while (keepAlive) {
            Context context;

            if (!HttpParser::parse(
                    clientSocket,
                    context.req,
                    keepAlive
                )) {
                break;
            }

            metro.handle(context);

            HttpWriter::write(
                clientSocket,
                context,
                keepAlive
            );
        }
    }
};
