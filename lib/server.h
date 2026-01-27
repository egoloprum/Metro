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
            if (!readRequest(clientSocket, context, keepAlive))
                break;

            metro.handle(context);
            writeResponse(clientSocket, context, keepAlive);
        }
    }

    bool readRequest(int clientSocket, Context& context, bool& keepAlive) {
        std::string requestBuffer;

        if (!readUntilHeaders(clientSocket, requestBuffer))
            return false;

        std::istringstream requestStream(requestBuffer);
        parseRequestLine(requestStream, context.req);
        parseHeaders(requestStream, context.req);

        keepAlive = isKeepAlive(context.req);
        parseBody(clientSocket, requestBuffer, context.req);

        return true;
    }

    bool readUntilHeaders(int clientSocket, std::string& requestBuffer) {
        char bufferChunk[BUFFER_SIZE];

        while (requestBuffer.find("\r\n\r\n") == std::string::npos) {
            ssize_t bytesRead =
                recv(clientSocket, bufferChunk, sizeof(bufferChunk), 0);

            if (bytesRead == 0) return false;

            if (bytesRead < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return false;

                perror("recv");
                return false;
            }

            requestBuffer.append(bufferChunk, bytesRead);

            if (requestBuffer.size() > 64 * 1024) {
                std::cerr << "Header too large\n";
                return false;
            }
        }
        return true;
    }

    void parseRequestLine(std::istream& input, Request& request) {
        std::string rawPath;
        std::string httpVersion;

        input >> request._method >> rawPath >> httpVersion;

        auto queryPos = rawPath.find('?');
        if (queryPos != std::string::npos) {
            request._path = rawPath.substr(0, queryPos);
            METRO_HELPERS::parseQuery(
                rawPath.substr(queryPos + 1),
                request._query
            );
        } else {
            request._path = rawPath;
        }
    }

    void parseHeaders(std::istream& input, Request& request) {
        std::string headerLine;
        std::getline(input, headerLine); // consume remainder

        while (std::getline(input, headerLine) && headerLine != "\r") {
            auto colonPos = headerLine.find(':');
            if (colonPos == std::string::npos) continue;

            std::string headerKey = headerLine.substr(0, colonPos);
            std::string headerValue = headerLine.substr(colonPos + 1);

            METRO_HELPERS::trim(headerValue);
            request._headers[headerKey] = headerValue;
        }
    }

    void parseBody(int clientSocket,
                   std::string& requestBuffer,
                   Request& request) {
        auto headerIt = request._headers.find("Content-Length");
        if (headerIt == request._headers.end()) return;

        size_t contentLength = std::stoul(headerIt->second);

        size_t headersEndPos =
            requestBuffer.find("\r\n\r\n") + 4;
        size_t alreadyRead =
            requestBuffer.size() - headersEndPos;

        request._body.clear();
        request._body.reserve(contentLength);

        if (alreadyRead > 0) {
            size_t bytesToCopy =
                std::min(alreadyRead, contentLength);
            request._body.append(
                requestBuffer.data() + headersEndPos,
                bytesToCopy
            );
        }

        while (request._body.size() < contentLength) {
            char bufferChunk[BUFFER_SIZE];
            ssize_t bytesRead = recv(
                clientSocket,
                bufferChunk,
                std::min(sizeof(bufferChunk),
                         contentLength - request._body.size()),
                0
            );

            if (bytesRead <= 0) break;
            request._body.append(bufferChunk, bytesRead);
        }
    }

    bool isKeepAlive(const Request& request) {
        auto headerIt = request._headers.find("Connection");
        if (headerIt == request._headers.end()) return true;
        return headerIt->second != "close";
    }

    void writeResponse(int clientSocket,
                       Context& context,
                       bool keepAlive) {
        std::ostringstream responseStream;

        responseStream
            << "HTTP/1.1 "
            << context.res._status << " "
            << METRO_HELPERS::reasonPhrase(context.res._status)
            << "\r\n";

        for (const auto& header : context.res._headers) {
            responseStream
                << header.first << ": "
                << header.second << "\r\n";
        }

        responseStream
            << "Content-Length: "
            << context.res._body.size() << "\r\n"
            << "Connection: "
            << (keepAlive ? "keep-alive" : "close") << "\r\n"
            << "Date: "
            << METRO_HELPERS::getCurrentDate()
            << "\r\n\r\n"
            << context.res._body;

        std::string responseString = responseStream.str();
        send(clientSocket,
             responseString.c_str(),
             responseString.size(),
             0);
    }
};
