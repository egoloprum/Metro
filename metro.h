#ifndef METRO_H
#define METRO_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include <vector>
#include <functional>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

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

using Handler = std::function<void(Context&)>;

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

class Server {
    Metro& metro;
    int port;

public:
    Server(Metro& metro, int port) : metro(metro), port(port) {}
    void listen() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        bind(sock, (sockaddr*)&addr, sizeof(addr));
        ::listen(sock, 10);

        std::cout << "Listening on port " << port << "\n";

        while (true) {
            int client = accept(sock, nullptr, nullptr);

            char buffer[4096];
            read(client, buffer, sizeof(buffer));

            std::istringstream req(buffer);
            std::string method, path;
            req >> method >> path;

            Context context;
            context.method = method;
            context.path = path;

            metro.handle(context);

            std::ostringstream res;
            res << "HTTP/1.1 " << context.status << " OK\r\n"
                << "Content-Length: " << context.response.size() << "\r\n"
                << "Content-Type: text/plain\r\n\r\n"
                << context.response;

            auto out = res.str();
            send(client, out.c_str(), out.size(), 0);
            close(client);
        }
    }
};

#endif
