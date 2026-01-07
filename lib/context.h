#pragma once 

#include <string>
#include <unordered_map>

struct Context {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> params;
    std::string body;

    int status = 200;
    std::string response;

    void text(const std::string& txt, int code = 200) {
        status = code;
        response = txt;
    }
};
