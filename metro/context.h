#pragma once 

#include <string>
#include <unordered_map>
#include <optional>
#include <fstream>
#include <ios>

#include "types.h"
#include "constants.h"

namespace Metro {
  using namespace Types;

  struct Request {
    public:

    std::string _method;
    std::string _path;

    Header  _headers;
    Body    _body;

    std::unordered_map<std::string, std::string> _params;
    std::unordered_map<std::string, std::vector<std::string>> _query;

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

    // const Body& body() const { 
    //   return _body; 
    // }

    // Body& body() { return _body; }

    const Text& text() const {
      return std::get<Text>(_body);
    }

    const Json& json() const {
      return std::get<Json>(_body);
    }

    const Form& form() const {
      return std::get<Form>(_body);
    }

    const Binary& binary() const {
      return std::get<Binary>(_body);
    }
  };

  // TODO: Implement response caching
  
  struct Response {
      int     _status = 200;
      Header  _headers;
      Body    _body;
  
      Response() = default;
  
      Response& status(int code) {
          _status = code;
          return *this;
      }
  
      Response& header(const std::string& key, const std::string& value) {
          _headers[key] = value;
          return *this;
      }
  
      Response& body(Body body) {
          _body = body;
          return *this;
      }
  
      Response& text(const std::string& txt, int code = -1) {
          if (code != -1) _status = code;
          _headers["Content-Type"] = "text/plain; charset=utf-8";
          _body = txt;
          return *this;
      }

      Response& json(Json data, int code = -1) {
        if (code != -1) _status = code;
        _headers["Content-Type"] = "application/json";
        _body = data;
        return *this;
      }

      Response& stream(Stream::Writer writer, size_t contentLength = 0, const std::string& contentType = "") {
        if (!contentType.empty()) {
          _headers["Content-Type"] = contentType;
        }
        if (contentLength > 0) {
          _headers["Content-Length"] = std::to_string(contentLength);
        } else {
          _headers["Transfer-Encoding"] = "chunked";
        }
        _body = Stream(std::move(writer), contentLength);
        return *this;
      }

      Response& file(const std::string& path, const std::string& contentType = "application/octet-stream") {
        // Implementation could optimize with sendfile syscall
        return stream([path](Stream::ChunkWriter write) {
          std::ifstream file(path, std::ios::binary);
          char buffer[8192];
          while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
            if (!write(buffer, file.gcount())) return false;
          }
          return true;
        }, 0, contentType); 
      }
  };
  
  struct Context {
      Request req;
      Response res;
  };
}
