#pragma once

#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <cstring>

#include "../context.h"
#include "../helpers.h"
#include "../constants.h"

namespace Metro {
  class HttpWriter {
  public:
      static void write(int clientSocket, const Context& context, bool keepAlive) {
          if (shouldUseChunkedEncoding(context)) {
              writeChunkedResponse(clientSocket, context, keepAlive);
          } else {
              writeStandardResponse(clientSocket, context, keepAlive);
          }
      }
      
  private:
      static bool shouldUseChunkedEncoding(const Context& context) {
          // Check if chunked encoding is requested and supported
          auto transferEncoding = context.res._headers.find(std::string(Constants::Http_Header::TRANSFER_ENCODING));
          if (transferEncoding != context.res._headers.end() &&
              transferEncoding->second.find("chunked") != std::string::npos) {
              return true;
          }
          
          // Auto-enable for streaming responses without content-length
          auto contentLength = context.res._headers.find(
              std::string(Constants::Http_Header::CONTENT_LENGTH));
          if (contentLength == context.res._headers.end() &&
              !context.res._body.empty()) {
              return true;
          }
          
          return false;
      }
      
      static void writeStandardResponse(int clientSocket, const Context& context, bool keepAlive) {
          std::string response = buildResponse(context, keepAlive);
          send(clientSocket, response.data(), response.size(), 0);
      }
      
      static void writeChunkedResponse(int clientSocket, const Context& context, bool keepAlive) {
          std::ostringstream headerStream;
          writeStatusLine(headerStream, context);
          
          // Replace content-length with transfer-encoding
          auto headers = context.res._headers;
          headers.erase(std::string(Constants::Http_Header::CONTENT_LENGTH));
          headers[std::string(Constants::Http_Header::TRANSFER_ENCODING)] = "chunked";
          
          // Write custom headers
          for (const auto& [headerName, headerValue] : headers) {
              headerStream << headerName << ": " << headerValue << "\r\n";
          }
          
          writeFixedHeaders(headerStream, context, keepAlive);
          headerStream << "\r\n";
          
          std::string headersStr = headerStream.str();
          send(clientSocket, headersStr.data(), headersStr.size(), 0);
          
          // Write body in chunks
          if (!context.res._body.empty()) {
              writeChunk(clientSocket, context.res._body);
          }
          
          writeLastChunk(clientSocket);
          writeTrailers(clientSocket, context);
      }
      
      static void writeChunk(int clientSocket, const std::string& data) {
          std::ostringstream chunkStream;
          
          chunkStream << std::hex << data.size() << "\r\n";
          chunkStream << data << "\r\n";
          
          std::string chunkStr = chunkStream.str();
          send(clientSocket, chunkStr.data(), chunkStr.size(), 0);
      }
      
      static void writeLastChunk(int clientSocket) {
          const char* lastChunk = "0\r\n\r\n";
          send(clientSocket, lastChunk, strlen(lastChunk), 0);
      }
      
      static void writeTrailers(int clientSocket, const Context& context) {
          // Look for trailers in headers (prefixed with X-Trailer-)
          std::ostringstream trailerStream;
          
          for (const auto& [headerName, headerValue] : context.res._headers) {
              if (headerName.find("X-Trailer-") == 0) {
                  std::string trailerName = headerName.substr(10);  // Remove "X-Trailer-" prefix
                  trailerStream << trailerName << ": " << headerValue << "\r\n";
              }
          }
          
          if (trailerStream.tellp() > 0) {
              std::string trailers = trailerStream.str();
              send(clientSocket, trailers.data(), trailers.size(), 0);
          }
      }
      
      static std::string buildResponse(const Context& context, bool keepAlive) {
          std::ostringstream responseStream;

          writeStatusLine(responseStream, context);
          writeHeaders(responseStream, context);
          writeFixedHeaders(responseStream, context, keepAlive);

          responseStream << "\r\n";
          
          if (!context.res._body.empty()) {
              responseStream << context.res._body;
          }
          
          return responseStream.str();
      }
      
      static void writeStatusLine(std::ostream& stream, const Context& context) {
          stream << "HTTP/1.1" << " "
                << context.res._status << " "
                << Helpers::reasonPhrase(context.res._status)
                << "\r\n";
      }
      
      static void writeHeaders(std::ostream& stream, const Context& context) {
          for (const auto& [headerName, headerValue] : context.res._headers) {
              // Skip X-Trailer- headers in regular headers
              if (headerName.find("X-Trailer-") != 0) {
                  stream << headerName << ": " << headerValue << "\r\n";
              }
          }
      }
      
      static void writeFixedHeaders(std::ostream& stream, const Context& context, bool keepAlive) {
          // Don't write content-length for chunked responses
          auto it = context.res._headers.find(std::string(Constants::Http_Header::TRANSFER_ENCODING));
          bool is_chunked = (it != context.res._headers.end() && it->second.find("chunked") != std::string::npos);

          if (!is_chunked) {
              stream << Constants::Http_Header::CONTENT_LENGTH << ": " 
                    << context.res._body.size() 
                    << "\r\n";
          }
          
          stream << Constants::Http_Header::CONNECTION << ": "
                << (keepAlive ? Constants::Http_Connection::KEEP_ALIVE 
                              : Constants::Http_Connection::CLOSE)
                << "\r\n";
          
          stream << Constants::Http_Header::DATE << ": "
                << Helpers::getCurrentDate()
                << "\r\n";
      }
  };
}
