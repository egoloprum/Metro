#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>

#include "metro.h"
#include "helpers.h"

constexpr int BACKLOG_SIZE = 10;
constexpr int BUFFER_SIZE  = 4096;

class Server {
    Metro& metro;
    int port;

public:
    Server(Metro& metro, int port) : metro(metro), port(port) {}

    void listen() {
        int sock = createSocket();
        if (sock < 0) return;

        std::cout << "Listening on port " << port << "\n";

        while (true) {
            int client = accept(sock, nullptr, nullptr);
            if (client < 0) continue;

            handleClient(client);
            close(client);
        }
    }

private:
    int createSocket() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "socket() failed\n";
            return -1;
        }

        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "bind() failed\n";
            close(sock);
            return -1;
        }

        if (::listen(sock, BACKLOG_SIZE) < 0) {
            std::cerr << "listen() failed\n";
            close(sock);
            return -1;
        }

        return sock;
    }

    void handleClient(int client) {
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytesRead = read(client, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) return;

        std::istringstream stream(buffer);
        Context context;

        parseRequestLine(stream, context);
        parseHeaders(stream, context);
        parseBody(stream, client, context, bytesRead);

        metro.handle(context);

        sendResponse(client, context);
    }

    void parseRequestLine(std::istringstream& stream, Context& ctx) {
        std::string rawPath, version;
        stream >> ctx.req.method >> rawPath >> version;

        auto qpos = rawPath.find('?');
        if (qpos != std::string::npos) {
            ctx.req.path = rawPath.substr(0, qpos);
            METRO_HELPERS::parseQuery(
                rawPath.substr(qpos + 1),
                ctx.req._query
            );
        } else {
            ctx.req.path = rawPath;
        }
    }

    void parseHeaders(std::istringstream& stream, Context& ctx) {
        std::string line;
        std::getline(stream, line); 

        while (std::getline(stream, line) && line != "\r") {
            auto colon = line.find(':');
            if (colon == std::string::npos) continue;

            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);

            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            ctx.req._headers[key] = value;
        }
    }

    void parseBody(
        std::istringstream& stream,
        int client,
        Context& ctx,
        ssize_t alreadyRead
    ) {
        auto it = ctx.req._headers.find("Content-Length");
        if (it == ctx.req._headers.end()) return;

        size_t contentLength = std::stoul(it->second);
        if (contentLength == 0) return;

        std::string body;
        body.resize(contentLength);

        size_t streamPos = stream.tellg();
        size_t buffered = alreadyRead - streamPos;
        size_t copied = 0;

        if (buffered > 0) {
            stream.read(&body[0], std::min(buffered, contentLength));
            copied = stream.gcount();
        }

        while (copied < contentLength) {
            ssize_t r = read(client, &body[copied], contentLength - copied);
            if (r <= 0) break;
            copied += r;
        }

        ctx.req._body = std::move(body);
    }

    void sendResponse(int client, Context& ctx) {
        std::ostringstream res;

        res << "HTTP/1.1 "
            << ctx.res._status << " "
            << METRO_HELPERS::reasonPhrase(ctx.res._status) << "\r\n";

        for (auto& h : ctx.res._headers) {
            res << h.first << ": " << h.second << "\r\n";
        }

        res << "Content-Length: " << ctx.res._body.size() << "\r\n";
        res << "Date: " << METRO_HELPERS::getCurrentDate() << "\r\n\r\n";
        res << ctx.res._body;

        auto out = res.str();
        send(client, out.c_str(), out.size(), 0);
    }
};
