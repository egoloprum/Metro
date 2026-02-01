#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "context.h"
#include "types.h"
#include "constants.h"
#include "helpers.h"
#include "http/http_error.h"
#include "route.h"

namespace Metro {
  using namespace Types; 

  class App {
    Router router_;
    std::vector<Middleware> middlewares_;

    void executeRoute(Context& context) {
      auto result = router_.matchRoute(context.req.getPath(), context.req.getMethod(), context);

      if (result.status == Router::MatchStatus::NotFound) {
        throw HttpError(
          Constants::Http_Status::NOT_FOUND,
          "Route not found: " + context.req.getPath()
        );
      } 
      else if (result.status == Router::MatchStatus::MethodNotAllowed) {
        std::string allowValue;
        for (size_t i = 0; i < result.allowedMethods.size(); ++i) {
          if (i > 0) allowValue += ", ";
          allowValue += result.allowedMethods[i];
        }

        context.res.header(Constants::Http_Header::ALLOW, allowValue);
        throw HttpError(
          Constants::Http_Status::METHOD_NOT_ALLOWED,
          "Method not allowed: " + context.req.getMethod()
        );
      }

      result.endpoint.handler(context);
    }

    void negotiateResponse(Context& context) {
      auto accept = context.req.header(Constants::Http_Header::ACCEPT);
      if (!accept || accept->empty()) return;

      auto it = context.res.getHeaders().find(Constants::Http_Header::CONTENT_TYPE);
      if (it == context.res.getHeaders().end()) return;

      if (accept->find("*/*") != std::string::npos) return;
      if (accept->find(it->second) != std::string::npos) return;

      throw HttpError(
        Constants::Http_Status::NOT_ACCEPTABLE,
        Helpers::reasonPhrase(Constants::Http_Status::NOT_ACCEPTABLE)
      );
    }

    void runMiddleware(Context& context) {
      size_t index = 0;
      
      Next next = [&]() {
        if (index < middlewares_.size()) {
          auto& middleware = middlewares_[index++];
          
          try {
            middleware(context, next);
            
            if (context.res.isCommitted()) {
              return;
            }
          } catch (...) {
            if (context.res.isCommitted()) {
              throw;
            }
            throw;
          }
        } else {
          try {
            executeRoute(context);
          } catch (...) {
            if (context.res.isCommitted()) {
              throw;
            }
            throw; 
          }
        }
      };
      
      try {
        next();
      } catch (const std::exception& e) {
        if (context.res.isCommitted()) {
          throw;
        }
        throw;
      }
    }

    public:
    App() = default;
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    App& use(Middleware middleware) {
      this->middlewares_.push_back(std::move(middleware));
      return *this;
    }

    RouteBuilder route(const std::string& path) {
      return RouteBuilder(router_, path);
    }

    App& get(const std::string& path, Handler handler) { 
      router_.addRoute(path, Constants::Http_Method::GET, std::move(handler));
      return *this;
    }
    App& post(const std::string& path, Handler handler) { 
      router_.addRoute(path, Constants::Http_Method::POST, std::move(handler));
      return *this;
    }
    App& put(const std::string& path, Handler handler) { 
      router_.addRoute(path, Constants::Http_Method::PUT, std::move(handler));
      return *this;
    }
    App& del(const std::string& path, Handler handler) { 
      router_.addRoute(path, Constants::Http_Method::DELETE, std::move(handler));
      return *this;
    }
    App& patch(const std::string& path, Handler handler) { 
      router_.addRoute(path, Constants::Http_Method::PATCH, std::move(handler));
      return *this;
    }
    App& head(const std::string& path, Handler handler) { 
      router_.addRoute(path, Constants::Http_Method::HEAD, std::move(handler));
      return *this;
    }
    App& options(const std::string& path, Handler handler) { 
      router_.addRoute(path, Constants::Http_Method::OPTIONS, std::move(handler));
      return *this;
    }
  
    void handle(Context& context) {
      negotiateResponse(context);
      runMiddleware(context);
    }
  };
}
