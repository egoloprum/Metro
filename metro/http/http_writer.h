#pragma once

#include <sstream>
#include <unistd.h>

#include "context.h"
#include "helpers.h"
#include "constants.h"

namespace Metro {
  class HttpWriter {
    public:

    static void write(int clientSocket, const Context& context, bool keepAlive) {
      std::string response = buildResponse(context, keepAlive);
      send(clientSocket, response.data(), response.size(), 0);
    }
  
    private:

    static std::string buildResponse(const Context& context, bool keepAlive) {
      std::ostringstream output;
        
      writeStatusLine(output, context);
      writeHeaders(output, context);
      writeFixedHeaders(output, context, keepAlive);
        
      output << "\r\n";
        
      if (!context.res._body.empty()) { output << context.res._body; }
        
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
  
    static void writeFixedHeaders(std::ostream& output, const Context& context, bool keepAlive) {
      auto it = context.res._headers.find(std::string(Constants::Http_Header::TRANSFER_ENCODING));
      bool is_chunked = (it != context.res._headers.end() && it->second.find("chunked") != std::string::npos);

      if (!is_chunked) {
        output  << Constants::Http_Header::CONTENT_LENGTH << ": " 
                << context.res._body.size() 
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
  };
}
