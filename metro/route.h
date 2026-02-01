#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <memory>
#include <functional>

#include "helpers.h"
#include "types.h"
#include "constants.h"
#include "http/http_error.h"

namespace Metro { struct Context; }

namespace Metro {
  using namespace Types;

  class Router {
  public:
    enum class MatchStatus { NotFound, MethodNotAllowed, Found };

    struct Endpoint {
      std::string method;
      Handler handler;
      std::vector<std::string> paramNames;
    };

    struct MatchResult {
      MatchStatus status;
      Endpoint endpoint;
      std::vector<std::string> allowedMethods; 
    };

    Router() : root_(std::make_unique<RouteNode>()) {}

    // Register a handler for specific path and method
    void addRoute(const std::string& path, const std::string& method, Handler handler) {
      std::vector<std::string> paramNames;
      RouteNode* node = root_.get();
      std::stringstream ss(path);
      std::string segment;

      while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue;

        if (segment[0] == ':') {
          std::string paramName = segment.substr(1);
          auto it = node->paramChildren.find(paramName);
          if (it == node->paramChildren.end()) {
            node->paramChildren[paramName] = std::make_unique<RouteNode>();
          }
          paramNames.push_back(paramName);
          node = node->paramChildren[paramName].get();
        } else {
          auto it = node->children.find(segment);
          if (it == node->children.end()) {
            node->children[segment] = std::make_unique<RouteNode>();
          }
          node = node->children[segment].get();
        }
      }

      node->endpoints[method] = Endpoint{method, std::move(handler), paramNames};
    }

    // Match a request path to an endpoint, populating context params
    MatchResult matchRoute(const std::string& path, const std::string& method, Context& context) {
      std::string normalized = Helpers::PathSanitizer::normalize(path);
      std::stringstream ss(normalized);
      std::string segment;
      std::vector<std::string> segments;

      while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) segments.push_back(segment);
      }

      Endpoint endpoint;
      std::vector<std::string> allowedMethods;
      auto status = resolveRoute(root_.get(), segments, 0, method, context, endpoint, &allowedMethods);

      return MatchResult{status, std::move(endpoint), std::move(allowedMethods)};
    }

  private:
    struct RouteNode {
      std::unordered_map<std::string, std::unique_ptr<RouteNode>> children;
      std::unordered_map<std::string, std::unique_ptr<RouteNode>> paramChildren;
      std::unordered_map<std::string, Endpoint> endpoints;
    };

    std::unique_ptr<RouteNode> root_;

    MatchStatus resolveRoute(
      RouteNode* node, const std::vector<std::string>& segments, size_t index,
      const std::string& method, Context& context, Endpoint& outEndpoint,
      std::vector<std::string>* allowedMethods
    ) {
      if (!node) return MatchStatus::NotFound;
      if (index > 100) { 
        throw HttpError(
          Constants::Http_Status::URI_TOO_LONG, 
          "Route recursion too deep"
        );
      }

      if (index == segments.size()) {
        auto it = node->endpoints.find(method);
        if (it != node->endpoints.end()) {
          outEndpoint = it->second;
          return MatchStatus::Found;
        }

        if (node->endpoints.empty()) {
          return MatchStatus::NotFound;
        }

        if (allowedMethods) {
          allowedMethods->clear();
          for (const auto& [method, _] : node->endpoints) {
            allowedMethods->push_back(method);
          }
        }
        return MatchStatus::MethodNotAllowed;
      }

      const std::string& seg = segments[index];

      auto childIt = node->children.find(seg);
      if (childIt != node->children.end()) {
        auto result = resolveRoute(childIt->second.get(), segments, index + 1, method, 
                                  context, outEndpoint, allowedMethods);
        if (result != MatchStatus::NotFound) return result;
      }

      MatchStatus bestResult = MatchStatus::NotFound;
      std::vector<std::string> allAllowed;

      for (auto& [paramName, childNode] : node->paramChildren) {
        context.req.getParams()[paramName] = Helpers::PathSanitizer::decodeSegment(seg);
        
        std::vector<std::string> branchAllowed;
        auto result = resolveRoute(childNode.get(), segments, index + 1, method,
                                  context, outEndpoint, &branchAllowed);

        if (result == MatchStatus::Found) {
          return MatchStatus::Found;
        } else if (result == MatchStatus::MethodNotAllowed) {
          bestResult = MatchStatus::MethodNotAllowed;
          allAllowed.insert(allAllowed.end(), branchAllowed.begin(), branchAllowed.end());
        }
        
        context.req.getParams().erase(paramName);
      }

      if (bestResult == MatchStatus::MethodNotAllowed && allowedMethods) {
        std::sort(allAllowed.begin(), allAllowed.end());
        auto last = std::unique(allAllowed.begin(), allAllowed.end());
        allAllowed.erase(last, allAllowed.end());
        *allowedMethods = std::move(allAllowed);
      }

      return bestResult;
    }
  };

  class RouteBuilder {
  public:
    RouteBuilder(Router& router, std::string path) 
      : router_(router), path_(std::move(path)) {}

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
      for (const auto& m : methods) {
        registerMethod(m, handler);
      }
      return *this;
    }

  private:
    Router& router_;
    std::string path_;

    void registerMethod(const std::string& method, Handler handler) {
      router_.addRoute(path_, method, std::move(handler));
    }
  };
}
