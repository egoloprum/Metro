#pragma once

#include <string>
#include <vector>
#include <regex>

#include "context.h"
#include "types.h"
#include "route.h"

class Metro {
    std::vector<Route> routes;
    std::vector<Handler> middleware;

public:
    Metro() = default;

    Metro(const Metro&) = delete;
    Metro& operator=(const Metro&) = delete;

    Metro(Metro&&) = delete;
    Metro& operator=(Metro&&) = delete;

    Metro& use(Handler m) {
        middleware.push_back(m);
        return *this;
    }

    Metro& get(const std::string& path, Handler h) {
        routes.push_back(makeRoute("GET", path, h));
        return *this;
    }

    Metro& post(const std::string& path, Handler h) {
        routes.push_back(makeRoute("POST", path, h));
        return *this;
    }

    void handle(Context& ctx) {
        for (auto& m : middleware) m(ctx);

        for (auto& r : routes) {
            if (r.method != ctx.method) continue;

            std::smatch match;
            if (std::regex_match(ctx.path, match, r.pathRegex)) {
                for (size_t i = 0; i < r.paramNames.size(); ++i)
                    ctx.params[r.paramNames[i]] = match[i + 1];

                r.handler(ctx);
                return;
            }
        }

        ctx.text("Not Found", 404);
    }
};
