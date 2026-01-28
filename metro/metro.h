#pragma once

#include <string>
#include <vector>
#include <regex>

#include "context.h"
#include "types.h"
#include "route.h"
#include "constants.h"
#include "helpers.h"

namespace Metro {
  using namespace Types; 

  class App {
      std::vector<Route> routes;
      std::vector<Middleware> middleware;
  
      void dispatchRoute(Context& context) {
          for (auto& route : this->routes) {
              if (route.method != context.req._method) continue;
  
              std::smatch match;
              if (std::regex_match(context.req._path, match, route.pathRegex)) {
                  for (size_t i = 0; i < route.paramNames.size(); ++i) {
                      context.req._params[route.paramNames[i]] = Helpers::urlDecode(match[i + 1].str());
                  }
  
                  route.handler(context);
                  return;
              }
          }
  
          context.res
            .status(Constants::Http_Status::NOT_FOUND)
            .text(Helpers::reasonPhrase(Constants::Http_Status::NOT_FOUND));
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
}

