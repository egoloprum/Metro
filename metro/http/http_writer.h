#pragma once

#include <sstream>
#include <unistd.h>

#include "context.h"
#include "helpers.h"
#include "constants.h"
#include "types.h"

namespace Metro {
  class HttpWriter {
    public:

    static void write(int clientSocket, const Context& context, bool keepAlive) {
      std::string response = buildResponse(context, keepAlive);
      sendResponse(clientSocket, response);
    }
  
    private:

    static std::string bodyToString(const Types::Body& body) {
      return std::visit([](const auto& content) -> std::string {
        using T = std::decay_t<decltype(content)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
          return "";
        } else if constexpr (std::is_same_v<T, std::string>) {
          return content;
        } else if constexpr (std::is_same_v<T, Types::Form>) {
          std::string result;
          for (const auto& [key, value] : content) {
            if (!result.empty()) result += '&';
            result += Helpers::urlEncode(key) + '=' + Helpers::urlEncode(value);
          }
          return result;
        } else if constexpr (std::is_same_v<T, Types::Json>) {
          return content.dump();
        } else if constexpr (std::is_same_v<T, Types::Binary>) {
          return std::string(content.begin(), content.end());
        } else {
          return "";
        }
      }, body);
    }

    static std::string buildResponse(const Context& context, bool keepAlive) {
      std::ostringstream output;
      std::string body_str = bodyToString(context.res._body);
        
      writeStatusLine(output, context);
      writeHeaders(output, context);
      writeFixedHeaders(output, context, body_str.size(), keepAlive);
        
      output << "\r\n";

      if (!body_str.empty()) { 
        output << body_str; 
      }
        
      return output.str();
    }
  
    static void writeStatusLine(std::ostream& output, const Context& context) {
      output  << "HTTP/1.1" << " "
              << context.res._status << " "
              << Helpers::reasonPhrase(context.res._status)
              << "\r\n";
    }
  
    static void writeHeaders(std::ostream& output, const Context& context) {
      for (const auto& [headerName, headerValue] : context.res._headers) {
        output  << headerName << ": " 
                << headerValue 
                << "\r\n";
      }
    }
  
    static void writeFixedHeaders(std::ostream& output, const Context& context, size_t body_str_size, bool keepAlive) {
      auto it = context.res._headers.find(std::string(Constants::Http_Header::TRANSFER_ENCODING));
      bool is_chunked = (it != context.res._headers.end() && it->second.find("chunked") != std::string::npos);

      if (!is_chunked) {
        output  << Constants::Http_Header::CONTENT_LENGTH << ": " 
                << body_str_size
                << "\r\n";
      }
      
      output  << Constants::Http_Header::CONNECTION << ": "
              << (keepAlive ? Constants::Http_Connection::KEEP_ALIVE 
                            : Constants::Http_Connection::CLOSE)
              << "\r\n";
      
      output  << Constants::Http_Header::DATE << ": "
              << Helpers::getCurrentDate()
              << "\r\n";
    }

    static void sendResponse(int clientSocket, std::string response) {
      const char* responseData = response.data();
      size_t remainingData = response.size();

      while (remainingData > 0) {
        ssize_t sentData = ::send(clientSocket, responseData, remainingData, 0);
        if (sentData <= 0) {
            return; 
        }
        responseData += sentData;
        remainingData -= sentData;
      }
    }
  };
}
