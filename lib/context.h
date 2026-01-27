#pragma once 

#include <string>
#include <unordered_map>
#include <optional>

#include "http/http_types.h"
#include "config.h"

namespace Metro {
  struct Request {
      std::string _method;
      std::string _path;
      std::string _original_path;
  
      Header _headers;
      std::string _body;
  
      std::unordered_map<std::string, std::string> _params;
      std::unordered_map<std::string, std::vector<std::string>> _query;

      bool _parsing_error = false;
      std::string _parsing_error_msg;
  
      Request() = default;
  
      std::optional<std::string> header(const std::string& key) const {
          auto it = _headers.find(key);
          if (it != _headers.end()) return it->second;
          return {};
      }
  
      std::string params(const std::string& key) const {
          auto it = _params.find(key);
          if (it != _params.end()) return it->second;
          return "";
      }
  
      std::string query(const std::string& key) const {
          auto it = _query.find(key);
          if (it == _query.end() || it->second.empty()) return "";
          return it->second[0];
      }
  
      const std::vector<std::string>& queries(const std::string& key) const {
          static const std::vector<std::string> empty;
          auto it = _query.find(key);
          return it == _query.end() ? empty : it->second;
      }
  
      const std::string& body() const { return _body; }
      std::string& body() { return _body; }

      bool hasParsingError() const { return _parsing_error; }
      const std::string& parsingErrorMessage() const { return _parsing_error_msg; }
      
      void setParsingError(const std::string& msg) {
          _parsing_error = true;
          _parsing_error_msg = msg;
      }
  };
  
  struct Response {
      int _status = 200;
      Header _headers;
      std::string _body;
  
      Response() = default;
  
      Response& status(int code) {
          _status = code;
          return *this;
      }
  
      Response& header(const std::string& key, const std::string& value) {
          _headers[key] = value;
          return *this;
      }
  
      Response& body(const std::string& b) {
          _body = b;
          return *this;
      }
  
      Response& text(const std::string& txt, int code = -1) {
          if (code != -1) _status = code;
          _headers["Content-Type"] = "text/plain; charset=utf-8";
          _body = txt;
          return *this;
      }

      Response& chunked(bool enable = true) {
          if (enable) {
              _headers["Transfer-Encoding"] = "chunked";
              _headers.erase("Content-Length");
          } else {
              _headers.erase("Transfer-Encoding");
          }
          return *this;
      }
      
      Response& trailer(const std::string& key, const std::string& value) {
          _headers["X-Trailer-" + key] = value;
          return *this;
      }
      
      Response& stream(const std::string& contentType = "application/octet-stream") {
          chunked(true);
          _headers["Content-Type"] = contentType;
          return *this;
      }
  };
  
  struct Context {
      Request req;
      Response res;

      const Config* config = nullptr; 

      bool isFeatureEnabled(const std::string& feature) const {
        if (!config) return false;
        return true;
    }
  };
}
