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

constexpr int BACKLOG_SIZE = 128;
constexpr int BUFFER_SIZE  = 8192;

class Server {
    Metro& metro;
    int port;

public:
    Server(Metro& m, int p) : metro(m), port(p) {}

    void listen() {
        int sock = createSocket();
        bindSocket(sock);
        startListen(sock);

        std::cout << "Listening on port " << port << "\n";

        while (true) {
            int client = accept(sock, nullptr, nullptr);
            if (client < 0) continue;

            handleConnection(client);
            close(client);
        }
    }

private:
    int createSocket() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        return sock;
    }

    void bindSocket(int sock) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            exit(1);
        }
    }

    void startListen(int sock) {
        if (::listen(sock, BACKLOG_SIZE) < 0) {
            perror("listen");
            exit(1);
        }
    }

    void setTimeout(int fd) {
        timeval tv{};
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }


    void handleConnection(int client) {
        setTimeout(client);

        bool keepAlive = true;
        while (keepAlive) {
            Context ctx;
            if (!readRequest(client, ctx, keepAlive)) break;
            metro.handle(ctx);
            writeResponse(client, ctx, keepAlive);
        }
    }


    bool readRequest(int client, Context& ctx, bool& keepAlive) {
        std::string buffer;

        if (!readUntilHeaders(client, buffer))
            return false;

        std::istringstream ss(buffer);
        parseRequestLine(ss, ctx.req);
        parseHeaders(ss, ctx.req);

        keepAlive = isKeepAlive(ctx.req);

        parseBody(client, buffer, ctx.req);
        return true;
    }


    bool readUntilHeaders(int client, std::string& buffer) {
        char temp[BUFFER_SIZE];

        while (buffer.find("\r\n\r\n") == std::string::npos) {
            ssize_t n = recv(client, temp, sizeof(temp), 0);
            if (n == 0) return false;          
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return false;
                }
                perror("recv"); 
                return false;
            }
            buffer.append(temp, n);

            if (buffer.size() > 64 * 1024) {  
                std::cerr << "Header too large\n";
                return false;
            }
        }
        return true;
    }


    void parseRequestLine(std::istream& in, Request& req) {
        std::string rawPath, version;
        in >> req._method >> rawPath >> version;

        auto qpos = rawPath.find('?');
        if (qpos != std::string::npos) {
            req._path = rawPath.substr(0, qpos);
            METRO_HELPERS::parseQuery(rawPath.substr(qpos + 1), req._query);
        } else {
            req._path = rawPath;
        }
    }

    void parseHeaders(std::istream& in, Request& req) {
        std::string line;
        std::getline(in, line);

        while (std::getline(in, line) && line != "\r") {
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;

            std::string k = line.substr(0, pos);
            std::string v = line.substr(pos + 1);

            METRO_HELPERS::trim(v);
            req._headers[k] = v;
        }
    }

    void parseBody(int client, std::string& buffer, Request& req) {
        auto it = req._headers.find("Content-Length");
        if (it == req._headers.end()) return;

        size_t len = std::stoul(it->second);

        size_t headerEnd = buffer.find("\r\n\r\n") + 4;
        size_t already = buffer.size() - headerEnd;

        req._body.clear();
        req._body.reserve(len);

        if (already > 0) {
            size_t take = std::min(already, len);
            req._body.append(buffer.data() + headerEnd, take);
        }

        while (req._body.size() < len) {
            char temp[BUFFER_SIZE];
            ssize_t n = recv(client, temp,
                std::min(sizeof(temp), len - req._body.size()),
                0
            );
            if (n <= 0) break;
            req._body.append(temp, n);
        }
    }


    bool isKeepAlive(const Request& req) {
        auto it = req._headers.find("Connection");
        if (it == req._headers.end()) return true;
        return it->second != "close";
    }

    void writeResponse(int client, Context& ctx, bool keepAlive) {
        std::ostringstream out;

        out << "HTTP/1.1 " << ctx.res._status
            << " " << METRO_HELPERS::reasonPhrase(ctx.res._status) << "\r\n";

        for (auto& h : ctx.res._headers)
            out << h.first << ": " << h.second << "\r\n";

        out << "Content-Length: " << ctx.res._body.size() << "\r\n";
        out << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n";
        out << "Date: " << METRO_HELPERS::getCurrentDate() << "\r\n\r\n";
        out << ctx.res._body;

        auto s = out.str();
        send(client, s.c_str(), s.size(), 0);
    }
};
