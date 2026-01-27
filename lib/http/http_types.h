#pragma once 

namespace Metro {
  struct CaseInsensitiveHash {
      size_t operator()(const std::string& key) const noexcept {
          size_t h = 0;
          for (char c : key)
              h = h * 131 + std::tolower(c);
          return h;
      }
  };
  
  struct CaseInsensitiveEqual {
      bool operator()(const std::string& a,
                      const std::string& b) const noexcept {
          if (a.size() != b.size()) return false;
          for (size_t i = 0; i < a.size(); ++i)
              if (std::tolower(a[i]) != std::tolower(b[i]))
                  return false;
          return true;
      }
  };
  
  using Header = std::unordered_map<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqual>;
}


