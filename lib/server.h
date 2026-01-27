// server.h - Modified
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
#include "config.h"  
#include "http/http_parser.h"
#include "http/http_writer.h"

namespace Metro {
class Server {
    App& app;
    Config config;
    
public:
    Server(App& appInstance, const Config& serverConfig)
        : app(appInstance), config(serverConfig) {}
    
    Server(App& appInstance, int listenPort)
        : app(appInstance) {
        config.setPort(listenPort);
    }
    
    void listen() {
        int serverSocket = createSocket();
        bindSocket(serverSocket);
        startListen(serverSocket);
        
        std::cout << "Metro server listening on port " << config.server().port << "\n";
        
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
        address.sin_port = htons(config.server().port);  
        
        if (bind(serverSocket,
                 reinterpret_cast<sockaddr*>(&address),
                 sizeof(address)) < 0) {
            perror("bind");
            exit(1);
        }
    }
    
    void startListen(int serverSocket) {
        if (::listen(serverSocket, config.server().backlog_size) < 0) {
            perror("listen");
            exit(1);
        }
    }
    
    void setTimeout(int socketFd) {
        timeval timeout{};
        timeout.tv_sec = config.server().timeout_seconds;
        timeout.tv_usec = 0;
        setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout));
        setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO,
                   &timeout, sizeof(timeout));
    }
    
    void handleConnection(int clientSocket) {
        setTimeout(clientSocket);
        bool keepAlive = config.features().enable_keep_alive;
        
        while (keepAlive) {
            Context context;
            context.config = &config;
            
            if (!HttpParser::parse(
                clientSocket,
                context,
                keepAlive,
                config  
            )) {
                break;
            }
            
            app.handle(context);
            
            HttpWriter::write(
                clientSocket,
                context,
                keepAlive
            );
        }
    }
};
}
