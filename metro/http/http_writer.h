#pragma once

#include <sstream>
#include <unistd.h>
#include <sys/uio.h> 
#include <chrono>
#include <ctime>
#include <iomanip>

#include "context.h"
#include "helpers.h"
#include "constants.h"
#include "types.h"

namespace Metro {
  class HttpWriter {
    public:

    static void write(int clientSocket, const Context& context, bool keepAlive) {
      auto bodyView = buildBodyView(context.res._body);
      std::string headers = buildHeaders(context, bodyView.size, keepAlive);

      if (bodyView.size > 0) {
        sendScatterResponse(clientSocket, headers, bodyView.data, bodyView.size);
      } else {
        sendAllResponse(clientSocket, headers.data(), headers.size());
      }
    }
  
    private:

    struct BodyView {
      const char* data;
      size_t size;
      std::string storage;
    };

    static BodyView buildBodyView(const Types::Body& body) {
      return std::visit([](const auto& content) -> BodyView {
        using T = std::decay_t<decltype(content)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
          return {nullptr, 0, {}};
        }
        else if constexpr (std::is_same_v<T, Types::Binary>) {
          return {
            reinterpret_cast<const char*>(content.data()), 
            content.size(), 
            {}
          };
        }
        else if constexpr (std::is_same_v<T, Types::Text>) {
          return {content.data(), content.size(), {}};
        }
        else if constexpr (std::is_same_v<T, Types::Json>) {
          BodyView view;
          view.storage = content.dump();
          view.data = view.storage.data();
          view.size = view.storage.size();
          return view;
        }
        else if constexpr (std::is_same_v<T, Types::Form>) {
          BodyView view;
          view.storage = transformFormToString(content);
          view.data = view.storage.data();
          view.size = view.storage.size();
          return view;
        }
        return {nullptr, 0, {}};
      }, body);
    }

    static std::string getCurrentDate() {
      auto now = std::chrono::system_clock::now();
      std::time_t now_time = std::chrono::system_clock::to_time_t(now);
      
      std::tm gmt_time;
      gmtime_r(&now_time, &gmt_time);
      
      std::stringstream date_string_stream;
      date_string_stream << std::put_time(&gmt_time, "%a, %d %b %Y %H:%M:%S GMT");
      return date_string_stream.str();
    }

    static std::string transformFormToString(const Types::Form& form) {
      std::string result;
      for (const auto& [key, value] : form) {
        if (!result.empty()) result += '&';
        result += Helpers::PathSanitizer::encodeSegment(key) + '=' + 
                  Helpers::PathSanitizer::encodeSegment(value);
      }
      return result;
    }

    static std::string buildHeaders(const Context& context, size_t bodySize, bool keepAlive) {
      std::ostringstream output;
        
      writeStatusLine(output, context);
      writeCustomHeaders(output, context);
      writeFixedHeaders(output, context, bodySize, keepAlive);
      output << "\r\n";
        
      return output.str();
    }
  
    static void writeStatusLine(std::ostream& output, const Context& context) {
      output  << "HTTP/1.1" << " "
              << context.res._status << " "
              << Helpers::reasonPhrase(context.res._status)
              << "\r\n";
    }
  
    static void writeCustomHeaders(std::ostream& output, const Context& context) {
      for (const auto& [headerName, headerValue] : context.res._headers) {
        output  << headerName << ": " 
                << headerValue 
                << "\r\n";
      }
    }
  
    static void writeFixedHeaders(std::ostream& output, const Context& context, size_t bodySize, bool keepAlive) {
      auto it = context.res._headers.find(std::string(Constants::Http_Header::TRANSFER_ENCODING));
      bool is_chunked = (it != context.res._headers.end() && it->second.find("chunked") != std::string::npos);

      if (!is_chunked) {
        output  << Constants::Http_Header::CONTENT_LENGTH << ": " 
                << bodySize
                << "\r\n";
      }
      
      output  << Constants::Http_Header::CONNECTION << ": "
              << (keepAlive ? Constants::Http_Connection::KEEP_ALIVE 
                            : Constants::Http_Connection::CLOSE)
              << "\r\n";
      
      output  << Constants::Http_Header::DATE << ": "
              << getCurrentDate()
              << "\r\n";
    }

    static void sendScatterResponse(int clientSocket, const std::string& headers, 
                           const char* bodyData, size_t bodySize) {
      struct iovec iov[2];
      iov[0].iov_base = const_cast<char*>(headers.data());
      iov[0].iov_len = headers.size();
      iov[1].iov_base = const_cast<char*>(bodyData);  // Zero-copy pointer
      iov[1].iov_len = bodySize;

      size_t total = headers.size() + bodySize;
      size_t sent_total = 0;

      while (sent_total < total) {
        ssize_t sent = writev(clientSocket, iov, 2);
        if (sent <= 0) {
          return; // Error or disconnect
        }
        sent_total += sent;

        // Adjust iovec for partial writes (kernel may not consume all)
        if (static_cast<size_t>(sent) < iov[0].iov_len) {
          // Partial write within headers
          iov[0].iov_base = static_cast<char*>(iov[0].iov_base) + sent;
          iov[0].iov_len -= sent;
        } else if (sent == static_cast<ssize_t>(iov[0].iov_len)) {
          // Headers fully sent, body starts next
          iov[0].iov_len = 0; // Mark as done
        } else {
          // Headers fully sent + partial body
          size_t body_sent = sent - iov[0].iov_len;
          iov[0].iov_len = 0;
          iov[1].iov_base = static_cast<char*>(iov[1].iov_base) + body_sent;
          iov[1].iov_len -= body_sent;
        }
      }
    }

    static void sendAllResponse(int clientSocket, const char* data, size_t length) {
      while (length > 0) {
        ssize_t sent = ::send(clientSocket, data, length, 0);
        if (sent <= 0) {
          return; 
        }
        data += sent;
        length -= sent;
      }
    }
  };
}
