#pragma once

#include <sstream>
#include <unistd.h>

#include "../context.h"
#include "../helpers.h"
#include "../constants.h"

class HttpParser {
    static constexpr size_t BUFFER_SIZE = 8192;
    static constexpr size_t MAX_HEADERS = 64 * 1024;

public:
    static inline bool parse(
        int clientSocket,
        Request& request,
        bool& keepAlive
    ) {
        std::string buffer;
        if (!readHeaders(clientSocket, buffer))
            return false;

        std::istringstream input(buffer);
        parseRequestLine(input, request);
        parseHeaders(input, request);

        keepAlive = shouldKeepAlive(request);
        parseBody(clientSocket, buffer, request);
        return true;
    }

private:
    static inline bool readHeaders(
        int clientSocket,
        std::string& buffer
    ) {
        char chunk[BUFFER_SIZE];

        while (buffer.find("\r\n\r\n") == std::string::npos) {
            ssize_t n = recv(clientSocket, chunk, sizeof(chunk), 0);
            if (n <= 0) return false;
            buffer.append(chunk, n);
            if (buffer.size() > MAX_HEADERS) return false;
        }
        return true;
    }

    static inline void parseRequestLine(
        std::istream& in,
        Request& req
    ) {
        std::string rawPath, version;
        in >> req._method >> rawPath >> version;

        auto q = rawPath.find('?');
        if (q != std::string::npos) {
            req._path = rawPath.substr(0, q);
            METRO_HELPERS::parseQuery(
                rawPath.substr(q + 1),
                req._query
            );
        } else {
            req._path = rawPath;
        }
    }

    static inline void parseHeaders(
        std::istream& in,
        Request& req
    ) {
        std::string line;
        std::getline(in, line);

        while (std::getline(in, line) && line != "\r") {
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            METRO_HELPERS::trim(val);

            req._headers[key] = val;
        }
    }

    static inline void parseBody(
        int clientSocket,
        const std::string& buffer,
        Request& req
    ) {
        auto lenHeader =
            req.header(std::string(HTTP_HEADER::CONTENT_LENGTH));

        if (!lenHeader) return;

        size_t len = std::stoul(*lenHeader);
        size_t headerEnd = buffer.find("\r\n\r\n") + 4;

        req._body.assign(
            buffer.data() + headerEnd,
            std::min(len, buffer.size() - headerEnd)
        );

        while (req._body.size() < len) {
            char chunk[BUFFER_SIZE];
            ssize_t n = recv(
                clientSocket,
                chunk,
                std::min(sizeof(chunk), len - req._body.size()),
                0
            );
            if (n <= 0) break;
            req._body.append(chunk, n);
        }
    }

    static inline bool shouldKeepAlive(
        const Request& req
    ) {
        auto conn =
            req.header(std::string(HTTP_HEADER::CONNECTION));
        if (!conn) return true;
        return *conn != HTTP_CONNECTION::CLOSE;
    }
};
