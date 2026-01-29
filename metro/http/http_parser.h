#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>

#include <unistd.h>
#include <sys/socket.h>

#include "context.h"
#include "helpers.h"
#include "constants.h"
#include "config.h"
#include "types.h"
#include "http_error.h"

namespace Metro {
  using namespace Types;

  class HttpLimits {
    public:
    explicit HttpLimits(const Config& config) : 
      max_body_size(config.security().max_body_size),
      max_headers_count(config.security().max_headers_count),
      max_query_params(config.security().max_query_params),
      max_header_size(config.server().max_header_size),
      max_buffer_size(config.server().max_buffer_size) {}

    const size_t max_body_size;
    const size_t max_headers_count;
    const size_t max_query_params;
    const size_t max_header_size;
    const size_t max_buffer_size;
  };

  class HttpHeaderReader {
    public:

    HttpHeaderReader(
      int clientSocket,
      std::string& buffer,
      size_t& total_bytes_read,
      const HttpLimits& limits
    ) : 
      clientSocket(clientSocket),
      buffer(buffer),
      total_bytes_read(total_bytes_read),
      limits(limits),
      chunk(limits.max_buffer_size) {}

    bool read() {
      while (!headersComplete()) {
        if (!receiveChunk()) return false;
        appendChunk();
        if (!checkLimits()) return false;
      }
      return true;
    }

    private:
    int clientSocket;
    std::string& buffer;
    size_t& total_bytes_read;
    const HttpLimits& limits;

    std::vector<char> chunk;
    ssize_t last_bytes_read = 0;

    bool headersComplete() const {
      return buffer.find("\r\n\r\n") != std::string::npos;
    }

    bool receiveChunk() {
      last_bytes_read = recv(clientSocket, chunk.data(), chunk.size(), 0);
      return last_bytes_read > 0;
    }

    void appendChunk() {
      buffer.append(chunk.data(), last_bytes_read);
      total_bytes_read += static_cast<size_t>(last_bytes_read);
    }

    bool checkLimits() const {
      if (buffer.size() > limits.max_header_size) {
        throw HttpError(
          Constants::Http_Status::REQUEST_HEADER_FIELDS_TOO_LARGE, 
          Helpers::reasonPhrase(Constants::Http_Status::REQUEST_HEADER_FIELDS_TOO_LARGE)
        );
      }
      if (total_bytes_read > limits.max_body_size) {
        throw HttpError(
          Constants::Http_Status::PAYLOAD_TOO_LARGE, 
          Helpers::reasonPhrase(Constants::Http_Status::PAYLOAD_TOO_LARGE)
        );
      };
      return true;
    }
  };

  class HttpRequestLineParser {
    public:
    explicit HttpRequestLineParser(const HttpLimits& limits)
      : limits(limits) {}

    bool parse(std::istream& input, Context& context) {
      if (!readRequestLine(input, context)) return false;
      if (!processPath(context)) return false;
      return true;
    }

    private:
    const HttpLimits& limits;
    std::string rawPath;

    bool readRequestLine(std::istream& input, Context& context) {
      std::string version;
      input >> context.req._method >> rawPath >> version;
      if (rawPath.empty()) {
        throw HttpError(
          Constants::Http_Status::BAD_REQUEST, 
          Helpers::reasonPhrase(Constants::Http_Status::BAD_REQUEST)
        );
      }
      return true;
    }

    bool processPath(Context& context) {
      rawPath = sanitizePath(rawPath);
      if (rawPath.empty()) {
        throw HttpError(
          Constants::Http_Status::BAD_REQUEST, 
          Helpers::reasonPhrase(Constants::Http_Status::BAD_REQUEST)
        );
      }

      auto query = rawPath.find('?');
      if (query == std::string::npos) {
        context.req._path = rawPath;
        return true;
      }

      context.req._path = rawPath.substr(0, query);
      return parseQueryString(rawPath.substr(query + 1), context);
    }

    bool parseQueryString(
      const std::string& queryString,
      Context& context
    ) {
      if (countQueryParams(queryString) > limits.max_query_params) {
        throw HttpError(
          Constants::Http_Status::URI_TOO_LONG, 
          Helpers::reasonPhrase(Constants::Http_Status::URI_TOO_LONG)
        );
      }

      Helpers::parseQuery(queryString, context.req._query);
      return true;
    }

    std::string sanitizePath(const std::string& path) {
      std::string sanitized = path;

      // Remove null bytes
      sanitized.erase(
        std::remove(sanitized.begin(), sanitized.end(), '\0'),
        sanitized.end()
      );

      // Decode URL encoding first
      sanitized = Helpers::urlDecode(sanitized);
      // Decode again and compare
      std::string doubleDecoded = Helpers::urlDecode(sanitized);

      if (doubleDecoded != sanitized && doubleDecoded.find("..") != std::string::npos) { return ""; }

      // Check for path traversal attempts
      if (sanitized.find("..") != std::string::npos) { return ""; }
      while (sanitized.find("/../") != std::string::npos) { return ""; }

      size_t pos;
      while ((pos = sanitized.find("/./")) != std::string::npos) {
        sanitized.erase(pos, 2);
      }

      // Remove trailing slash except for root
      if (sanitized.size() > 1 && sanitized.back() == '/') {
        sanitized.pop_back();
      }

      // Ensure path starts with /
      if (sanitized.empty() || sanitized[0] != '/') {
        sanitized = "/" + sanitized;
      }

      return sanitized;
    }

    size_t countQueryParams(const std::string& queryString) {
      if (queryString.empty()) return 0;
      return std::count(queryString.begin(), queryString.end(), '&') + 1;
    }
  };

  class HttpHeadersParser {
    public:
    explicit HttpHeadersParser(const HttpLimits& limits)
      : limits(limits) {}

    bool parse(std::istream& input, Context& context) {
      skipRequestLine(input);

      while (readNextHeaderLine(input)) {
        if (!validateHeaderCount()) { return false; }
        storeHeader(context);
      }
      return true;
    }

    private:
    const HttpLimits& limits;
    std::string line;
    size_t header_count = 0;

    void skipRequestLine(std::istream& input) {
      std::getline(input, line);
    }

    bool readNextHeaderLine(std::istream& input) {
      if (!std::getline(input, line)) return false;
      return line != "\r";
    }

    bool validateHeaderCount() {
      if (++header_count > limits.max_headers_count) {
        throw HttpError(
          Constants::Http_Status::REQUEST_HEADER_FIELDS_TOO_LARGE, 
          Helpers::reasonPhrase(Constants::Http_Status::REQUEST_HEADER_FIELDS_TOO_LARGE)
        );
      }
      return true;
    }

    void storeHeader(Context& context) {
      auto pos = line.find(':');
      if (pos == std::string::npos) return;

      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      trim(value);

      context.req._headers[key] = value;
    }

    inline void trim(std::string& value) {
      constexpr char space_char = ' ';
      constexpr char tab_char = '\t';
      constexpr char carriage_return_char = '\r';
      constexpr char newline_char = '\n';
      
      // Left trim
      size_t trim_start_position = 0;
      while (trim_start_position < value.size()) {
        const char current_char = value[trim_start_position];
        if (current_char != space_char && 
          current_char != tab_char && 
          current_char != carriage_return_char && 
          current_char != newline_char) {
          break;
        }
        ++trim_start_position;
      }
      value.erase(0, trim_start_position);
        
      // Right trim
      size_t trim_end_position = value.size();
      while (trim_end_position > 0) {
        const char current_char = value[trim_end_position - 1];
        if (current_char != space_char && 
          current_char != tab_char && 
          current_char != carriage_return_char && 
          current_char != newline_char) {
          break;
        }
        --trim_end_position;
      }
      value.erase(trim_end_position);
    }
  };

  class HttpBodyParser {
    public:
    HttpBodyParser(int clientSocket, const std::string& buffer, const HttpLimits& limits)
      : clientSocket(clientSocket),
        buffer(buffer),
        limits(limits) {}

    bool parse(Context& context) {
      if (!validateTransferEncoding(context)) return false;
      if (!readInitialBody(context)) return false;
      if (!readRemainingBody(context)) return false;

      context.req._body = parseBody(context);

      return true;
    }

    private:
    int clientSocket;
    std::string rawBody;
    const std::string& buffer;
    const HttpLimits& limits;

    bool validateTransferEncoding(Context& context) {
      auto transferEncoding =
        context.req.header(Constants::Http_Header::TRANSFER_ENCODING);

      if (transferEncoding && *transferEncoding == "chunked") {
        throw HttpError(
          Constants::Http_Status::LENGTH_REQUIRED, 
          Helpers::reasonPhrase(Constants::Http_Status::LENGTH_REQUIRED)
        );
      }
      return true;
    }

    bool readInitialBody(Context& context) {
      auto contentLengthHeader =
        context.req.header(Constants::Http_Header::CONTENT_LENGTH);

      if (!contentLengthHeader) {
        rawBody.clear();

        if (buffer.find("\r\n\r\n") + 4 < buffer.size()) {
          throw HttpError(
            Constants::Http_Status::UNSUPPORTED_MEDIA_TYPE,
            "Missing Content-Type"
          );
        }

        return true;
      }

      try {
        size_t content_length = std::stoul(*contentLengthHeader);
        if (content_length > limits.max_body_size) {
          throw HttpError(
            Constants::Http_Status::PAYLOAD_TOO_LARGE, 
            Helpers::reasonPhrase(Constants::Http_Status::PAYLOAD_TOO_LARGE)
          );
        }

        size_t header_end = buffer.find("\r\n\r\n") + 4;
        rawBody.assign(
          buffer.data() + header_end,
          std::min(content_length, buffer.size() - header_end)
        );

        return true;
      }
      catch (const std::invalid_argument& e) {
        throw HttpError(
          Constants::Http_Status::BAD_REQUEST, 
          Helpers::reasonPhrase(Constants::Http_Status::BAD_REQUEST)
        );
      } catch (const std::out_of_range& e) {
        throw HttpError(
          Constants::Http_Status::PAYLOAD_TOO_LARGE, 
          Helpers::reasonPhrase(Constants::Http_Status::PAYLOAD_TOO_LARGE)
        );
      }
    }

    bool readRemainingBody(Context& context) {
      auto contentLengthHeader =
        context.req.header(Constants::Http_Header::CONTENT_LENGTH);
      if (!contentLengthHeader) return true;

      try {
        size_t content_length = std::stoul(*contentLengthHeader);
        std::vector<char> chunk(limits.max_buffer_size);
  
        while (rawBody.size() < content_length) {
          ssize_t bytes_read = recv(clientSocket, chunk.data(), chunk.size(), 0);
  
          if (bytes_read <= 0) break;
  
          rawBody.append(chunk.data(), bytes_read);
  
          if (rawBody.size() > limits.max_body_size) {
            throw HttpError(
              Constants::Http_Status::PAYLOAD_TOO_LARGE, 
              Helpers::reasonPhrase(Constants::Http_Status::PAYLOAD_TOO_LARGE)
            );
          }
        }

        return true;
      }
      catch (const std::exception& e) {
        throw HttpError(
          Constants::Http_Status::BAD_REQUEST, 
          Helpers::reasonPhrase(Constants::Http_Status::BAD_REQUEST)
        );
      }
    }

    Body parseBody(Context& context) {
      auto contentType = context.req.header(Constants::Http_Header::CONTENT_TYPE);

      if (rawBody.empty()) {
        if (!contentType) {
          return Text{};
        }

        if (contentType->find(Constants::Http_Content_Type::APPLICATION_JSON) != std::string::npos) {
          return Json{}; 
        }

        if (contentType->find(Constants::Http_Content_Type::APPLICATION_FORM_URLENCODED) != std::string::npos) {
          return Form{};
        }

        if (contentType->find(Constants::Http_Content_Type::TEXT) != std::string::npos ||
            contentType->find(Constants::Http_Content_Type::APPLICATION_JAVASCRIPT) != std::string::npos) {
          return Text{};
        }

        return Binary{};
      }

      if (!contentType) {
        throw HttpError(
          Constants::Http_Status::UNSUPPORTED_MEDIA_TYPE,
          Helpers::reasonPhrase(Constants::Http_Status::UNSUPPORTED_MEDIA_TYPE)
        );
      }

      if (contentType->find(Constants::Http_Content_Type::APPLICATION_JSON) != std::string::npos) {
        try {
          return Json::parse(rawBody);
        } catch (...) {
          throw HttpError(
            Constants::Http_Status::BAD_REQUEST,
            "Malformed JSON body"
          );
        }
      }

      if (contentType->find(Constants::Http_Content_Type::APPLICATION_FORM_URLENCODED) != std::string::npos) {
        Form form;
        std::unordered_map<std::string, std::vector<std::string>> temp;
        Helpers::parseQuery(rawBody, temp);
        for (auto& [key, value] : temp) {
          if (!value.empty()) form[key] = value[0];
        }
        return form;
      }

      if (contentType->find(Constants::Http_Content_Type::TEXT) != std::string::npos ||
          contentType->find(Constants::Http_Content_Type::APPLICATION_JAVASCRIPT) != std::string::npos) {
        return rawBody;
      }

      return Binary(rawBody.begin(), rawBody.end());
    }
  };

  class HttpParser {
    static inline bool shouldKeepAlive(Context& context) {
      auto conn = context.req.header(Constants::Http_Header::CONNECTION);
      
      if (conn && *conn != Constants::Http_Connection::CLOSE && 
          *conn != Constants::Http_Connection::KEEP_ALIVE &&
          *conn != Constants::Http_Connection::UPGRADE) {
        throw HttpError(
          Constants::Http_Status::BAD_REQUEST, 
          Helpers::reasonPhrase(Constants::Http_Status::BAD_REQUEST)
        );
      }
      
      return true;
    }

    public:
    static inline bool parse(
      int clientSocket,
      Context& context,
      const Config& config
    ) {
      std::string buffer;
      size_t total_bytes_read = 0;

      try {  
        HttpLimits limits(config);
  
        HttpHeaderReader headerReader(
          clientSocket,
          buffer,
          total_bytes_read,
          limits
        );
        
        if (!headerReader.read()) return false;
  
        std::istringstream input(buffer);
  
        HttpRequestLineParser requestLineParser(limits);
        if (!requestLineParser.parse(input, context)) { return false; }
  
        HttpHeadersParser headersParser(limits);
        if (!headersParser.parse(input, context)) { return false; }
  
        if (!shouldKeepAlive(context)) { return false; }
  
        HttpBodyParser bodyParser(clientSocket, buffer, limits);
        if (!bodyParser.parse(context)) { return false; }
 
        return true;
      } catch (const HttpError& e) {
        context.res
          .status(e.status())
          .text(e.what());

        // HttpWriter::write(clientSocket, context, false);
        return false;
      }
    }
  };
} 
