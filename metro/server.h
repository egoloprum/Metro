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
  
    void setTimeout(int socketFd) {
      timeval timeout{};
      timeout.tv_sec = config.server().timeout_seconds;
      timeout.tv_usec = 0;
  
      if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Warning: Failed to set receive timeout on socket: " 
                  << std::strerror(errno) << std::endl;
      }

      if (setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Warning: Failed to set send timeout on socket: " 
                  << std::strerror(errno) << std::endl;
      }
    }

    void handleConnection(int clientSocket) {
      setTimeout(clientSocket);

      bool keepAlive = config.security().enable_keep_alive;

      while (keepAlive) {
        Context context;

        if (!HttpParser::parse(clientSocket, context, config)) { 
          if (context.res._status >= 400) {
            HttpWriter::write(clientSocket, context, false);
          }

          break;
        }

        try {
          app.handle(context);
        } catch (const HttpError& e) {
          context.res.status(e.status()).text(e.what());
        } catch (const std::exception& e) {
          context.res
            .status(Constants::Http_Status::INTERNAL_SERVER_ERROR)
            .text(Helpers::reasonPhrase(Constants::Http_Status::INTERNAL_SERVER_ERROR));
          std::cerr << "Error handling request: " << e.what() << std::endl;
        }

        HttpWriter::write(clientSocket, context, keepAlive);

        auto connHeader = context.req.header(Constants::Http_Header::CONNECTION);
        if (connHeader && *connHeader == Constants::Http_Connection::CLOSE) {
          keepAlive = false;
        }
      }

      close(clientSocket);
    }
  };
}

