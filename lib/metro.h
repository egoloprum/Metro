#pragma once

#include <string>
#include <vector>
#include <regex>

#include "context.h"
#include "types.h"
#include "route.h"
#include "constants.h"
#include "helpers.h"

class Metro {
    std::vector<Route> routes;
    std::vector<Middleware> middleware;

    void dispatchRoute(Context& context) {
        for (auto& route : this->routes) {
            if (route.method != context.req._method) continue;

            std::smatch match;
            if (std::regex_match(context.req._path, match, route.pathRegex)) {
                for (size_t i = 0; i < route.paramNames.size(); ++i) {
                    context.req._params[route.paramNames[i]] = METRO_HELPERS::urlDecode(match[i + 1].str());
                }

                route.handler(context);
                return;
            }
        }

        context.res.status(404).text("Not Found");
    }

public:
    Metro() = default;

    Metro(const Metro&) = delete;
    Metro& operator=(const Metro&) = delete;

    Metro(Metro&&) = delete;
    Metro& operator=(Metro&&) = delete;

    Metro& use(Middleware middleware) {
        this->middleware.push_back(std::move(middleware));
        return *this;
    }

    Metro& get(const std::string& path, Handler handler) {
        this->routes.push_back(makeRoute(HTTP_METHOD::GET, path, handler));
        return *this;
    }

    Metro& post(const std::string& path, Handler handler) {
        this->routes.push_back(makeRoute(HTTP_METHOD::POST, path, handler));
        return *this;
    }

    Metro& patch(const std::string& path, Handler handler) {
        this->routes.push_back(makeRoute(HTTP_METHOD::PATCH, path, handler));
        return *this;
    }

    Metro& put(const std::string& path, Handler handler) {
        this->routes.push_back(makeRoute(HTTP_METHOD::PUT, path, handler));
        return *this;
    }

    Metro& del(const std::string& path, Handler handler) {
        this->routes.push_back(makeRoute(HTTP_METHOD::DELETE, path, handler));
        return *this;
    }

    Metro& options(const std::string& path, Handler handler) {
        this->routes.push_back(makeRoute(HTTP_METHOD::OPTIONS, path, handler));
        return *this;
    }

    Metro& head(const std::string& path, Handler handler) {
        this->routes.push_back(makeRoute(HTTP_METHOD::HEAD, path, handler));
        return *this;
    }

    void handle(Context& context) {
        size_t index = 0;

        Next next = [&]() {
            if (index < this->middleware.size()) {
                auto& middleware = this->middleware[index++];
                middleware(context, next);
            } else {
                dispatchRoute(context);
            }
        };

        next();
    }
};
