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

    //// Add connection keep-alive with proper timeout

    void handleConnection(int clientSocket) {
      SocketGuard guard(clientSocket); 

      try {
        setTimeout(clientSocket);
      }
      catch (...) {
        std::cerr << "Fatal error in connection thread." << std::endl;
        return;
      }

      bool keepAlive = config.security().enable_keep_alive;

      while (keepAlive) {
        Context context;

        try {
          if (!HttpParser::parse(clientSocket, context, config)) break;
        } catch (HttpError& e) {
          context.res
            .status(e.status())
            .text(e.what());
          std::cerr << "Error parsing request: " << e.what() << std::endl;
          
          HttpWriter::write(clientSocket, context, false);
          break;
        }

        try {
          app.handle(context);

        } catch (const HttpError& e) {
          context.res
            .status(e.status())
            .text(e.what());
          std::cerr << "Error handling request: " << e.what() << std::endl;

        } catch (const std::bad_variant_access& e) {
          context.res
            .status(Constants::Http_Status::UNSUPPORTED_MEDIA_TYPE)
            .text(Helpers::reasonPhrase(Constants::Http_Status::UNSUPPORTED_MEDIA_TYPE));
          std::cerr << "Error handling request: " << e.what() << std::endl;

        } catch (const std::exception& e) {
          context.res
            .status(Constants::Http_Status::INTERNAL_SERVER_ERROR)
            .text(Helpers::reasonPhrase(Constants::Http_Status::INTERNAL_SERVER_ERROR));
          std::cerr << "Error handling request: " << e.what() << std::endl;
        }

        HttpWriter::write(clientSocket, context, keepAlive);

        //// keep-alive is ignored

        auto connHeader = context.req.header(Constants::Http_Header::CONNECTION);
        if (connHeader && *connHeader == Constants::Http_Connection::CLOSE) {
          keepAlive = false;
        }
      }
    }
  };
}

// DOCS: read about linux network sockets
