#pragma once

#include <sstream>
#include <unistd.h>

#include "../context.h"
#include "../helpers.h"
#include "../constants.h"

namespace Metro {
  class HttpWriter {
  public:
      static void write(int clientSocket, const Context& context, bool keepAlive) {
          std::string response = buildResponse(context, keepAlive);
          send(clientSocket, response.data(), response.size(), 0);
      }
  
  private:
      static std::string buildResponse(const Context& context, bool keepAlive) {
          std::ostringstream responseStream;
          
          writeStatusLine(responseStream, context);
          writeHeaders(responseStream, context);
          writeFixedHeaders(responseStream, context, keepAlive);
          
          // Empty line to separate headers from body
          responseStream << "\r\n";
          
          // Response body
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
              stream << headerName << ": " << headerValue << "\r\n";
          }
      }
  
      static void writeFixedHeaders(std::ostream& stream, const Context& context, bool keepAlive) {
          stream << Constants::Http_Header::CONTENT_LENGTH << ": " 
                 << context.res._body.size() 
                 << "\r\n";
          
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
