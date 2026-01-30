#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

#include "context.h"
#include "types.h"
#include "constants.h"
#include "helpers.h"
#include "http/http_error.h"

namespace Metro {
  using namespace Types; 

  class App {
    enum class MatchResult { NotFound, MethodNotAllowed, Found };

    struct Endpoint {
      std::string method;
      Handler handler;
      std::vector<std::string> paramNames;
    };

    struct RouteNode {
      RouteNode* paramChild = nullptr;
      std::string paramName;
      std::unordered_map<std::string, Endpoint> endpoints;
      std::unordered_map<std::string, RouteNode*> children;

      ~RouteNode() { 
        for (auto& p : children) delete p.second; 
        delete paramChild; 
      }
    };
    
    RouteNode rootNode;
    std::vector<Middleware> middlewares;

    class RouteBuilder {
      App& app;
      std::string path;
      
    public:
      RouteBuilder(App& app, std::string path) : app(app), path(std::move(path)) {}
      
      RouteBuilder(const RouteBuilder&) = delete;
      RouteBuilder& operator=(const RouteBuilder&) = delete;
      RouteBuilder(RouteBuilder&&) = default;
      RouteBuilder& operator=(RouteBuilder&&) = default;

      RouteBuilder& get(Handler handler) { 
        registerMethod(Constants::Http_Method::GET, std::move(handler)); 
        return *this; 
      }
      
      RouteBuilder& post(Handler handler) { 
        registerMethod(Constants::Http_Method::POST, std::move(handler)); 
        return *this; 
      }
      
      RouteBuilder& put(Handler handler) { 
        registerMethod(Constants::Http_Method::PUT, std::move(handler)); 
        return *this; 
      }
      
      RouteBuilder& del(Handler handler) { 
        registerMethod(Constants::Http_Method::DELETE, std::move(handler)); 
        return *this; 
      }
      
      RouteBuilder& patch(Handler handler) { 
        registerMethod(Constants::Http_Method::PATCH, std::move(handler)); 
        return *this; 
      }
      
      RouteBuilder& head(Handler handler) { 
        registerMethod(Constants::Http_Method::HEAD, std::move(handler)); 
        return *this; 
      }
      
      RouteBuilder& options(Handler handler) { 
        registerMethod(Constants::Http_Method::OPTIONS, std::move(handler)); 
        return *this; 
      }

      RouteBuilder& method(const std::string& httpMethod, Handler handler) {
        registerMethod(httpMethod, std::move(handler));
        return *this;
      }

      RouteBuilder& methods(std::initializer_list<std::string> methods, Handler handler) {
        for (const auto& method : methods) {
          registerMethod(method, handler);
        }
        return *this;
      }

      private:

      void registerMethod(const std::string& method, Handler handler) {
        std::vector<std::string> paramNames;
        app.registerRoute(path, method, std::move(handler), paramNames);
      }
    };

    RouteNode* registerRoute(const std::string& path, const std::string& method, 
                      Handler handler, std::vector<std::string>& paramNames) {
      RouteNode* node = &rootNode;
      std::stringstream ss(path);
      std::string segment;

      while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue;

        if (segment[0] == ':') {
          if (!node->paramChild) {
            node->paramChild = new RouteNode();
            node->paramChild->paramName = segment.substr(1);
          }
          paramNames.push_back(segment.substr(1));
          node = node->paramChild;
        } else {
          if (node->children.find(segment) == node->children.end()) {
            node->children[segment] = new RouteNode();
          }
          node = node->children[segment];
        }
      }

      node->endpoints.emplace(method, Endpoint{method, handler, paramNames});
      return node;
    }

    MatchResult resolveRoute(RouteNode* node, const std::vector<std::string>& segments, size_t index, 
                          Context& context, Endpoint& outEndpoint, 
                          std::vector<std::string>* allowedMethods = nullptr) {
      if (!node) return MatchResult::NotFound;
      if (index > 100) {
        throw HttpError(Constants::Http_Status::URI_TOO_LONG, "Path too deep");
      }

      if (index == segments.size()) {
        auto it = node->endpoints.find(context.req._method);
        if (it != node->endpoints.end()) {
          outEndpoint = it->second;
          return MatchResult::Found;
        }
        
        if (allowedMethods) {
          allowedMethods->clear();
          for (const auto& [method, _] : node->endpoints) {
            allowedMethods->push_back(method);
          }
        }
        return MatchResult::MethodNotAllowed;
      }

      const std::string& seg = segments[index];
      
      auto childIt = node->children.find(seg);
      if (childIt != node->children.end()) {
        auto result = resolveRoute(childIt->second, segments, index + 1, context, outEndpoint, allowedMethods);
        if (result != MatchResult::NotFound) return result;
      }

      if (node->paramChild) {
        context.req._params[node->paramChild->paramName] = Helpers::PathSanitizer::decodeSegment(seg);
        auto result = resolveRoute(node->paramChild, segments, index + 1, context, outEndpoint, allowedMethods);
        if (result != MatchResult::NotFound) return result;
        context.req._params.erase(node->paramChild->paramName);
      }

      return MatchResult::NotFound;
    }

    void executeRoute(Context& context) {
      std::stringstream ss(context.req._path);
      std::string segment;
      std::vector<std::string> segments;

      while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) segments.push_back(segment);
      }

      Endpoint endpoint;
      std::vector<std::string> allowedMethods;
      auto result = resolveRoute(&rootNode, segments, 0, context, endpoint, &allowedMethods);

      if (result == MatchResult::NotFound) {
        throw HttpError(Constants::Http_Status::NOT_FOUND,
                       "Route not found: " + context.req._path);
      } 
      else if (result == MatchResult::MethodNotAllowed) {
        std::string allowValue;
        for (size_t i = 0; i < allowedMethods.size(); ++i) {
          if (i > 0) allowValue += ", ";
          allowValue += allowedMethods[i];
        }

        context.res.header(Constants::Http_Header::ALLOW, allowValue);
        throw HttpError(Constants::Http_Status::METHOD_NOT_ALLOWED,
                       "Method not allowed: " + context.req._method);
      }

      endpoint.handler(context);
    }

    void negotiateResponse(Context& context) {
      auto accept = context.req.header(Constants::Http_Header::ACCEPT);
      if (!accept || accept->empty()) return;

      auto it = context.res._headers.find(Constants::Http_Header::CONTENT_TYPE);
      if (it == context.res._headers.end()) return;

      if (accept->find("*/*") != std::string::npos) return;
      if (accept->find(it->second) != std::string::npos) return;

      throw HttpError(Constants::Http_Status::NOT_ACCEPTABLE,
                     Helpers::reasonPhrase(Constants::Http_Status::NOT_ACCEPTABLE));
    }

    void runMiddleware(Context& context) {
      size_t index = 0;
      Next next = [&]() {
        if (index < middlewares.size()) {
          middlewares[index++](context, next);
        } else {
          executeRoute(context);
        }
      };
      next();
    }

  public:
    App() = default;
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    App& use(Middleware middleware) {
      this->middlewares.push_back(std::move(middleware));
      return *this;
    }

    RouteBuilder route(const std::string& path) {
      return RouteBuilder(*this, path);
    }

    App& get(const std::string& path, Handler handler) { 
      route(path).get(std::move(handler));
      return *this;
    }
    App& post(const std::string& path, Handler handler) { 
      route(path).post(std::move(handler));
      return *this;
    }
    App& put(const std::string& path, Handler handler) { 
      route(path).put(std::move(handler));
      return *this;
    }
    App& del(const std::string& path, Handler handler) { 
      route(path).del(std::move(handler));
      return *this;
    }
    App& patch(const std::string& path, Handler handler) { 
      route(path).patch(std::move(handler));
      return *this;
    }
    App& head(const std::string& path, Handler handler) { 
      route(path).head(std::move(handler));
      return *this;
    }
    App& options(const std::string& path, Handler handler) { 
      route(path).options(std::move(handler));
      return *this;
    }
  
    void handle(Context& context) {
      negotiateResponse(context);
      runMiddleware(context);
    }
  };
}
