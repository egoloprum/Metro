#pragma once 

#include <string>
#include <unordered_map>

// using Handler = std::function<void(Context&)>;

struct Context {
    /* ================= REQUEST ================= */

    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> params;

    std::unordered_map<std::string, std::string> requestHeaders;
    std::string requestBody;

    /* ================= RESPONSE ================= */

    int statusCode = 200;
    std::string responseBody;
    std::unordered_map<std::string, std::string> responseHeaders;
    
    Context& status(int code) {
        statusCode = code;
        return *this;
    }

    Context& header(const std::string& key, const std::string& value) {
        responseHeaders[key] = value;
        return *this;
    }

    Context& body(const std::string& body) {
        responseBody = body;
        return *this;
    }

    void text(const std::string& txt, int code = 200) {
        statusCode = code;
        responseHeaders["Content-Type"] = "text/plain ; charset=utf-8";
        responseBody = txt;
    }

    void json(const std::string& jsonStr, int code = 200) {
        statusCode = code;
        responseHeaders["Content-Type"] = "application/json";
        responseBody = jsonStr;
    }
};
