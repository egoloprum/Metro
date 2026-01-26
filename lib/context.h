#pragma once 

#include <string>
#include <unordered_map>
#include <optional>

struct Request {
    std::string method;
    std::string path;

    std::unordered_map<std::string, std::string> _headers;
    std::string _body;

    std::unordered_map<std::string, std::string> _params;
    std::unordered_map<std::string, std::string> _query; 

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
        if (it != _query.end()) return it->second;
        return "";
    }

    std::string body() const { return _body; }
};

struct Response {
    int _status = 200;
    std::unordered_map<std::string, std::string> _headers;
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
};

struct Context {
    Request req;
    Response res;

    bool responded = false;
};
