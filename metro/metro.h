#pragma once

#include <string>
#include <vector>
#include <regex>

#include "context.h"
#include "types.h"
#include "route.h"
#include "constants.h"
#include "helpers.h"
#include "http/http_error.h"

namespace Metro {
  using namespace Types; 

  class App {
    std::vector<Route> routes;
    std::vector<Middleware> middleware;

    void dispatchRoute(Context& context) {
      bool pathMatched = false;
      bool methodMatched = false;

      for (auto& route : this->routes) {
        std::smatch match;

        // TODO: Using std::regex for every route match is computationally expensive
        // Replace std::regex with radix tree for routing

        if (!std::regex_match(context.req._path, match, route.pathRegex)) {
          continue;
        }

        pathMatched = true;

        if (route.method != context.req._method) {
          continue;
        }

        methodMatched = true;

        for (size_t i = 0; i < route.paramNames.size(); ++i) {
          context.req._params[route.paramNames[i]] =
            Helpers::urlDecode(match[i + 1].str());
        }

        route.handler(context);
        return;
      }
      
      if (pathMatched && !methodMatched) {
        throw HttpError(
          Constants::Http_Status::METHOD_NOT_ALLOWED,
          Helpers::reasonPhrase(Constants::Http_Status::METHOD_NOT_ALLOWED)
        );
      }

      throw HttpError(
        Constants::Http_Status::NOT_FOUND,
        "Route not found: " + context.req._path
      );
    }

    void negotiateResponse(Context& context) {
      auto accept = context.req.header(Constants::Http_Header::ACCEPT);
      if (!accept || accept->empty()) return;

      auto it = context.res._headers.find(Constants::Http_Header::CONTENT_TYPE);
      if (it == context.res._headers.end()) return;

      const std::string& contentType = it->second;

      if (accept->find("*/*") != std::string::npos) return;
      if (accept->find(contentType) != std::string::npos) return;

      throw HttpError(
        Constants::Http_Status::NOT_ACCEPTABLE,
        Helpers::reasonPhrase(Constants::Http_Status::NOT_ACCEPTABLE)
      );
    }

    public:

    App() = default;

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    App(App&&) = delete;
    App& operator=(App&&) = delete;

    App& use(Middleware middleware) {
      this->middleware.push_back(std::move(middleware));
      return *this;
    }
  
    App& get(const std::string& path, Handler handler) {
      this->routes.push_back(makeRoute(Constants::Http_Method::GET, path, handler));
      return *this;
    }
  
    App& post(const std::string& path, Handler handler) {
      this->routes.push_back(makeRoute(Constants::Http_Method::POST, path, handler));
      return *this;
    }

    App& patch(const std::string& path, Handler handler) {
      this->routes.push_back(makeRoute(Constants::Http_Method::PATCH, path, handler));
      return *this;
    }

    App& put(const std::string& path, Handler handler) {
      this->routes.push_back(makeRoute(Constants::Http_Method::PUT, path, handler));
      return *this;
    }

    App& del(const std::string& path, Handler handler) {
      this->routes.push_back(makeRoute(Constants::Http_Method::DELETE, path, handler));
      return *this;
    }

    App& options(const std::string& path, Handler handler) {
      this->routes.push_back(makeRoute(Constants::Http_Method::OPTIONS, path, handler));
      return *this;
    }

    App& head(const std::string& path, Handler handler) {
      this->routes.push_back(makeRoute(Constants::Http_Method::HEAD, path, handler));
      return *this;
    }
  
    void handle(Context& context) {
      size_t index = 0;

      Next next = [&]() {
        if (index < middleware.size()) {
          middleware[index++](context, next);
        } else {
          dispatchRoute(context);
        }
      };

      negotiateResponse(context);

      next();
    }
  };
}
