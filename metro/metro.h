#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "context.h"
#include "types.h"
#include "constants.h"
#include "helpers.h"
#include "http/http_error.h"

namespace Metro {
  using namespace Types; 

  class App {
    struct Route {
      std::string method;
      Handler handler;
      std::vector<std::string> paramNames;
    };

    struct Node {
      Node* paramChild = nullptr;
      std::string paramName;
      std::unordered_map<std::string, Route> routes;
      std::unordered_map<std::string, Node*> children;

      ~Node() { for (auto& p : children) delete p.second; delete paramChild; }
    };
    
    Node root;
    std::vector<Middleware> middleware;

    Node* insertRoute(const std::string& path, const std::string& method, Handler handler, std::vector<std::string>& paramNames) {
      Node* node = &root;
      std::stringstream ss(path);
      std::string segment;

      while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue;

        if (segment[0] == ':') {
          if (!node->paramChild) {
            node->paramChild = new Node();
            node->paramChild->paramName = segment.substr(1);
          }
          paramNames.push_back(segment.substr(1));
          node = node->paramChild;
        } else {
          if (node->children.find(segment) == node->children.end()) {
            node->children[segment] = new Node();
          }
          node = node->children[segment];
        }
      }

      node->routes.emplace(method, Route{method, handler, paramNames});
      return node;
    }

    bool matchRoute(Node* node, const std::vector<std::string>& segments, size_t index, Context& context, Route& outRoute) {
      if (!node) return false;

      if (index == segments.size()) {
        auto it = node->routes.find(context.req._method);
        if (it != node->routes.end()) {
          outRoute = it->second;
          return true;
        }
        return false;
      }

      const std::string& seg = segments[index];
      if (node->children.find(seg) != node->children.end()) {
        if (matchRoute(node->children[seg], segments, index + 1, context, outRoute)) return true;
      }

      if (node->paramChild) {
        context.req._params[node->paramChild->paramName] = Helpers::urlDecode(seg);
        if (matchRoute(node->paramChild, segments, index + 1, context, outRoute)) return true;
        context.req._params.erase(node->paramChild->paramName);
      }

      return false;
    }

    void dispatchRoute(Context& context) {
      std::stringstream ss(context.req._path);
      std::string segment;
      std::vector<std::string> segments;

      while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) segments.push_back(segment);
      }

      Route route;
      if (!matchRoute(&root, segments, 0, context, route)) {
        throw HttpError(
          Constants::Http_Status::NOT_FOUND,
          "Route not found: " + context.req._path
        );
      }

      route.handler(context);
    }

    // TODO: explicit accept handling

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
      std::vector<std::string> tmp; 
      insertRoute(path, Constants::Http_Method::GET, handler, tmp); 
      return *this; 
    }

    App& post(const std::string& path, Handler handler) { 
      std::vector<std::string> tmp; 
      insertRoute(path, Constants::Http_Method::POST, handler, tmp); 
      return *this; 
    }

    App& patch(const std::string& path, Handler handler) { 
      std::vector<std::string> tmp; 
      insertRoute(path, Constants::Http_Method::PATCH, handler, tmp); 
      return *this; 
    }

    App& put(const std::string& path, Handler handler) { 
      std::vector<std::string> tmp; 
      insertRoute(path, Constants::Http_Method::PUT, handler, tmp); 
      return *this; 
    }

    App& del(const std::string& path, Handler handler) { 
      std::vector<std::string> tmp; 
      insertRoute(path, Constants::Http_Method::DELETE, handler, tmp); 
      return *this; 
    }

    App& options(const std::string& path, Handler handler) { 
      std::vector<std::string> tmp; 
      insertRoute(path, Constants::Http_Method::OPTIONS, handler, tmp); 
      return *this; 
    }

    App& head(const std::string& path, Handler handler) { 
      std::vector<std::string> tmp; 
      insertRoute(path, Constants::Http_Method::HEAD, handler, tmp); 
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
