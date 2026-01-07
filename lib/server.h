#pragma once 

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

#include "metro.h"

class Server {
    Metro& metro;
    int port;

public:
    Server(Metro& metro, int port) : metro(metro), port(port) {}
    void listen() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        bind(sock, (sockaddr*)&addr, sizeof(addr));
        ::listen(sock, 10);

        std::cout << "Listening on port " << port << "\n";

        while (true) {
            int client = accept(sock, nullptr, nullptr);

            char buffer[4096];
            read(client, buffer, sizeof(buffer));

            std::istringstream req(buffer);
            std::string method, path;
            req >> method >> path;

            Context context;
            context.method = method;
            context.path = path;

            metro.handle(context);

            std::ostringstream res;
            res << "HTTP/1.1 " << context.status << " OK\r\n"
                << "Content-Length: " << context.response.size() << "\r\n"
                << "Content-Type: text/plain\r\n\r\n"
                << context.response;

            auto out = res.str();
            send(client, out.c_str(), out.size(), 0);
            close(client);
        }
    }
};
