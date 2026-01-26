#pragma once 

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "metro.h"
#include "helpers.h"

constexpr const int BACKLOG_SIZE = 10;
constexpr const int BUFFER_SIZE = 4096;

class Server {
    Metro& metro;
    int port;

public:
    Server(Metro& metro, int port) : metro(metro), port(port) {}
    
    void listen() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Failed to create socket\n";
            return;
        }

        // Allow socket reuse
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "Failed to bind to port " << port << "\n";
            close(sock);
            return;
        }
        
        if (::listen(sock, BACKLOG_SIZE) < 0) {
            std::cerr << "Failed to listen on socket\n";
            close(sock);
            return;
        }

        std::cout << "Listening on port " << port << "\n";

        while (true) {
            int client = accept(sock, nullptr, nullptr);
            if (client < 0) {
                std::cerr << "Failed to accept connection\n";
                continue;
            }

            char buffer[BUFFER_SIZE] = {0};
            ssize_t bytes_read = read(client, buffer, sizeof(buffer) - 1);
            
            if (bytes_read > 0) {
                std::istringstream req(buffer);
                std::string method, path, version;
                req >> method >> path >> version;

                // Parse headers
                std::string line;
                std::getline(req, line); // Skip the rest of the first line
                
                Context context;
                context.req.method = method;
                context.req.path = path;
                
                // Parse headers
                while (std::getline(req, line) && line != "\r") {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string key = line.substr(0, colon);
                        std::string value = line.substr(colon + 1);

                        value.erase(0, value.find_first_not_of(" \t\r\n"));
                        value.erase(value.find_last_not_of(" \t\r\n") + 1);

                        context.req._headers[key] = value;
                    }
                }

                metro.handle(context);

                std::ostringstream response;
                response << "HTTP/1.1 "
                    << context.res._status << " "
                    << METRO_HELPERS::reasonPhrase(context.res._status) << "\r\n";

                for (auto& header : context.res._headers) {
                    response << header.first << ": " << header.second << "\r\n";
                }

                response << "Content-Length: " << context.res._body.size() << "\r\n";
                response << "Date: " << METRO_HELPERS::getCurrentDate() << "\r\n";
                
                response << "\r\n";
                response << context.res._body;

                auto out = response.str();
                ssize_t bytes_sent = send(client, out.c_str(), out.size(), 0);
                if (bytes_sent < 0) {
                    std::cerr << "Failed to send response\n";
                }
            }
            
            close(client);
        }
        
        close(sock);
    }
};
