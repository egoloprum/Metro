#pragma once 

#include <string>
#include <regex>
#include <vector>

#include "types.h"

struct Route {
    std::string method;
    std::regex pathRegex;
    std::vector<std::string> paramNames;
    Handler handler;
};

inline Route makeRoute(
    const std::string& method,
    const std::string& path,
    Handler h
) {
    std::string regexPath;
    std::vector<std::string> params;

    std::stringstream ss(path);
    std::string segment;

    while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue;

        regexPath += "/";
        if (segment[0] == ':') {
            params.push_back(segment.substr(1));
            regexPath += "([^/]+)";
        } else {
            regexPath += segment;
        }
    }

    if (regexPath.empty()) regexPath = "/";

    return {
        method,
        std::regex("^" + regexPath + "$"),
        params,
        h
    };
}
