#pragma once 

#include <string>
#include <unordered_map>
#include <optional>

#include "types.h"
#include "constants.h"
#include "http/http_error.h"

namespace Metro {
  using namespace Types;

  struct Request {
    private:

    template <typename T>
    T& expectBody(const char* expectedType) {
      if (!std::holds_alternative<T>(_body)) {
        throw HttpError(
          Constants::Http_Status::UNSUPPORTED_MEDIA_TYPE,
          std::string("Invalid body type, expected ") + expectedType
        );
      }
      return std::get<T>(_body);
    }

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

    Text& text() {
      return expectBody<Text>("text");
    }

    Json& json() {
      return expectBody<Json>("json");
    }

    Form& form() {
      return expectBody<Form>("form");
    }

    Binary& binary() {
      return expectBody<Binary>("binary");
    }

    const Text& text() const {
      return const_cast<Request*>(this)->text();
    }

    const Json& json() const {
      return const_cast<Request*>(this)->json();
    }

    const Form& form() const {
      return const_cast<Request*>(this)->form();
    }

    const Binary& binary() const {
      return const_cast<Request*>(this)->binary();
    }
  };
  
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
  };
  
  struct Context {
      Request req;
      Response res;
  };
}
