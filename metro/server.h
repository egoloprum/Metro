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
    int port;
    Config config;
  
    public:

    Server(App& appInstance, int listenPort) : app(appInstance), port(listenPort) {}
  
    void listen() {
      int serverSocket = createSocket();
      SocketGuard serverGuard(serverSocket);

      bindSocket(serverSocket);
      startListen(serverSocket);

      std::cout << "Listening on port " << port << "\n";

      while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) continue;

        handleConnection(clientSocket);
      }
    }
  
    private:
    class SocketGuard {
      private:
      int fileDescriptor;

      public:
      SocketGuard(int socketFileDescriptor) : fileDescriptor(socketFileDescriptor) {}
      ~SocketGuard() { if (fileDescriptor >= 0) close(fileDescriptor); }
      SocketGuard(const SocketGuard&) = delete;
      SocketGuard& operator=(const SocketGuard&) = delete;
    };

    int createSocket() {
      int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

      if (serverSocket < 0) {
        throw std::system_error(
          std::error_code(errno, std::system_category()),
          "Failed to create socket"
        );
      }

      int reuseAddress = 1;

      if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)) < 0) {
        close(serverSocket);
        throw std::system_error(
          std::error_code(errno, std::system_category()),
          "Failed to set socket option SO_REUSEADDR"
        );
      }

      return serverSocket;
    }
  
    void bindSocket(int serverSocket) {
      sockaddr_in address{};
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons(port);

      if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(serverSocket);
        throw std::system_error(
          std::error_code(errno, std::system_category()),
          "Failed to bind socket to port " + std::to_string(port)
        );
      }
    }
  
    void startListen(int serverSocket) {
      if (::listen(serverSocket, config.server().backlog_size) < 0) {
        close(serverSocket);
        throw std::system_error(
          std::error_code(errno, std::system_category()),
          "Failed to listen on socket"
        );
      }
    }
  
    void setTimeout(int socketFileDescriptor) {
      timeval timeout{};
      timeout.tv_sec = config.server().timeout_seconds;
      timeout.tv_usec = 0;
  
      if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Warning: Failed to set receive timeout on socket: " 
                  << std::strerror(errno) << std::endl;
      }

      if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Warning: Failed to set send timeout on socket: " 
                  << std::strerror(errno) << std::endl;
      }
    }

    void handleConnection(int clientSocket) {
      SocketGuard guard(clientSocket);
      setTimeout(clientSocket);

      auto lastActivity = std::chrono::steady_clock::now();
      const auto maxKeepAliveDuration = std::chrono::seconds(config.server().keep_alive_timeout_seconds);
      const size_t maxRequestsPerConnection = config.server().max_keep_alive_requests;

      size_t requestCount = 0;
      bool connectionOpen = true;
      bool keepAlive      = false;

      while (connectionOpen) {
        auto now = std::chrono::steady_clock::now();
        if (now - lastActivity > maxKeepAliveDuration) {
          break;
        }

        Context context;
        bool    parseSuccess = false;

        try {
          parseSuccess = HttpParser::parse(clientSocket, context, config);
        } catch (HttpError& e) {
          context.res
            .status(e.status())
            .text(e.what());

          HttpWriter::write(clientSocket, context, keepAlive);
          break;
        }

        if (!parseSuccess) break;

        lastActivity = std::chrono::steady_clock::now();
        requestCount++;

        try {
          app.handle(context);
        } catch (const HttpError& e) {
          context.res
            .status(e.status())
            .text(e.what());
        } catch (const std::exception& e) {
          context.res
            .status(Constants::Http_Status::INTERNAL_SERVER_ERROR)
            .text(Helpers::reasonPhrase(Constants::Http_Status::INTERNAL_SERVER_ERROR));
        }

        keepAlive = shouldKeepAlive(context, requestCount, maxRequestsPerConnection);
        
        HttpWriter::write(clientSocket, context, keepAlive);
        
        if (!keepAlive) {
          connectionOpen = false;
        }
      }
    }

    bool shouldKeepAlive(const Context& context, size_t requestCount, size_t maxRequests) {
      if (requestCount >= maxRequests) {
        return false;
      }

      auto connHeader = context.req.header(Constants::Http_Header::CONNECTION);
      const std::string& version = context.req.getHttpVersion();

      if (version == "1.1") {
        if (connHeader && *connHeader == Constants::Http_Connection::CLOSE) {
          return false;
        }
        return true;
      }

      if (version == "1.0") {
        if (connHeader && *connHeader == Constants::Http_Connection::KEEP_ALIVE) {
          return true;
        }
        return false;
      }

      return false;
    }
  };
}

//// DOCS: read about linux network sockets
