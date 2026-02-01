#pragma once 

#include <string>
#include <unordered_map>
#include <optional>
#include <fstream>
#include <ios>
#include <stdexcept>

#include "types.h"
#include "constants.h"

namespace Metro {
  class App;
  class HttpParser;
  class HttpWriter;

  using namespace Types;

  class Request {
  private:
    std::string method_;
    std::string path_;
    Header headers_;
    Body body_;
    std::unordered_map<std::string, std::string> params_;
    std::unordered_map<std::string, std::vector<std::string>> queries_;

    std::string http_version_ = "1.1";

  public:
    std::optional<std::string> header(const std::string& key) const {
      auto it = headers_.find(key);
      if (it != headers_.end()) return it->second;
      return std::nullopt;
    }
    
    std::string params(const std::string& key) const {
      auto it = params_.find(key);
      if (it != params_.end()) return it->second;
      return {};
    }
    
    std::string query(const std::string& key) const {
      auto it = queries_.find(key);
      if (it == queries_.end() || it->second.empty()) return {};
      return it->second[0];
    }
    
    const std::vector<std::string>& queries(const std::string& key) const {
      static const std::vector<std::string> empty;
      auto it = queries_.find(key);
      return it == queries_.end() ? empty : it->second;
    }
    
    const Text& text() const      { return std::get<Text>(body_); }
    const Json& json() const      { return std::get<Json>(body_); }
    const Form& form() const      { return std::get<Form>(body_); }
    const Binary& binary() const  { return std::get<Binary>(body_); }
    const Stream& stream() const  { return std::get<Stream>(body_); }

  private:
    friend class App;           
    friend class HttpParser;    
    friend class HttpRequestLineParser;
    friend class HttpHeadersParser;
    friend class HttpBodyParser;
    friend class Middlewares;
    friend class Server;
    friend class Router;

    const std::string& getHttpVersion() const noexcept { return http_version_; }
    const std::string& getMethod()      const noexcept { return method_; }
    const std::string& getPath()        const noexcept { return path_; }
    const Header& getHeaders()          const noexcept { return headers_; }
    const Body& getBody()               const noexcept { return body_; }

    void setHeader(std::string key, std::string value)  { headers_[key] = std::move(value); }
    void setHttpVersion(std::string version)            { http_version_ = std::move(version); }
    void setMethod(std::string method)                  { method_ = std::move(method); }
    void setPath(std::string path)                      { path_ = std::move(path); }
    void setBody(Body body)                             { body_ = std::move(body); }
    
    std::unordered_map<std::string, std::string>& getParams()               { return params_; }
    std::unordered_map<std::string, std::vector<std::string>>& getQueries() { return queries_; }
  };

  class Response {
  private:
    int status_ = 200;
    Header headers_;
    Body body_;
    bool committed_ = false;

  public:
    Response() = default;
    
    Response& status(int code) {
      checkNotCommitted();
      status_ = code;
      return *this;
    }
    
    Response& header(const std::string& key, const std::string& value) {
      checkNotCommitted();
      headers_[key] = value;
      return *this;
    }
    
    std::optional<std::string> header(const std::string& key) const {
      auto it = headers_.find(key);
      if (it != headers_.end()) return it->second;
      return std::nullopt;
    }
    
    Response& body(Body b) {
      checkNotCommitted();
      body_ = std::move(b);
      return *this;
    }
    
    Response& text(const std::string& txt, int code = -1) {
      if (code != -1) status(code);
      headers_["Content-Type"] = "text/plain; charset=utf-8";
      body_ = txt;
      return *this;
    }

    Response& json(const Json& data, int code = -1) {
      if (code != -1) status(code);
      headers_["Content-Type"] = "application/json";
      body_ = data;
      return *this;
    }

    Response& stream(Stream::Writer writer, size_t contentLength = 0, const std::string& contentType = "") {
      checkNotCommitted();
      if (!contentType.empty()) {
        headers_["Content-Type"] = contentType;
      }
      if (contentLength > 0) {
        headers_["Content-Length"] = std::to_string(contentLength);
      } else {
        headers_["Transfer-Encoding"] = "chunked";
      }
      body_ = Stream(std::move(writer), contentLength);
      return *this;
    }

    Response& file(const std::string& path, const std::string& contentType = "application/octet-stream") {
      return stream([path](Stream::ChunkWriter write) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return false; 
        char buffer[8192];
        while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
          if (!write(buffer, file.gcount())) return false;
        }
        return true;
      }, 0, contentType); 
    }
    
  private:
    friend class App;        
    friend class HttpWriter;  
    friend class Middlewares;
    
    void commit() { committed_ = true; }

    bool isCommitted()          const noexcept { return committed_; }
    int getStatus()             const noexcept { return status_; }
    const Header& getHeaders()  const noexcept { return headers_; }
    const Body& getBody()       const noexcept { return body_; }
    
    void checkNotCommitted() const {
      if (committed_) {
        throw std::runtime_error("Cannot modify response: already committed (headers sent)");
      }
    }
  };
  
  struct Context {
    Request req;
    Response res;
  };
}
