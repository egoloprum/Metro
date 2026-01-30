#pragma once

#include <functional>
#include <variant> 

#include "context.h"
#include "../lib/json.hpp"

namespace Metro {
  struct Context;
}

namespace Metro {
  namespace Types {
    using Next        = std::function<void()>;
    using Handler     = std::function<void(Context&)>;
    using Middleware  = std::function<void(Context&, Next)>;

    using Text        = std::string;
    using Form        = std::unordered_map<std::string, std::string>;
    using Json        = nlohmann::json;
    using Binary      = std::vector<std::uint8_t>;

    using Body        = std::variant<
      std::monostate,                   // not parsed / empty
      Text,                             // text/plain, text/html, etc
      Form,                             // application/x-www-form-urlencoded
      Json,                             // application/json
      Binary                            // images, pdf, octet-stream
    >;

    struct CaseInsensitiveHash {
      size_t operator()(const std::string& key) const noexcept {
        size_t h = 0;
        for (unsigned char uc : key) {  
          if (uc >= 'A' && uc <= 'Z') { uc |= 0x20; }
          h = h * 131 + uc;
        }
        return h;
      }
    };

    struct CaseInsensitiveEqual {
      bool operator()(const std::string& a, const std::string& b) const noexcept {
        if (a.size() != b.size()) return false;
        
        for (size_t i = 0; i < a.size(); ++i) {
          unsigned char ca = static_cast<unsigned char>(a[i]);
          unsigned char cb = static_cast<unsigned char>(b[i]);
          
          if (ca >= 'A' && ca <= 'Z') ca |= 0x20;
          if (cb >= 'A' && cb <= 'Z') cb |= 0x20;
          
          if (ca != cb) return false;
        }
        return true;  
      }
    };
    
    using Header = std::unordered_map<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqual>;
  }
}

