#pragma once

namespace Metro {
  class Config {
    public: 

    // Server Configuration
    struct ServerConfig {
      int port                = 3000;
      int timeout_seconds     = 5;
      int backlog_size        = 128;
      size_t max_buffer_size  = 8192;
      size_t max_header_size  = 64 * 1024;
    };

    // Security Configuration
    struct SecurityConfig {
      size_t max_body_size              = 10 * 1024 * 1024; // 10MB default
      bool enable_path_sanitization     = true;
      bool reject_double_encoded_paths  = true;
      size_t max_query_params           = 100;
      size_t max_headers_count          = 100;
    };

    // Static configuration loading
    static Config fromJson(const std::string& jsonPath);
    static Config fromEnv();

    // Getters
    const ServerConfig&   server() const { return server_config; }
    const SecurityConfig& security() const { return security_config; }

    // Setters (Fluent API)
    Config& setPort(int port) { server_config.port = port; return *this; }
    Config& setTimeoutSeconds(int seconds) { server_config.timeout_seconds = seconds; return *this; }
    Config& setMaxBodySize(size_t size) { security_config.max_body_size = size; return *this; }
    Config& enablePathSanitization(bool enable = true) { 
      security_config.enable_path_sanitization = enable; 
      return *this; 
    }

    private: 
    ServerConfig server_config;
    SecurityConfig security_config;
  };

  // class ConfigBuilder {
  //   public:
  //   static ConfigBuilder create() { return ConfigBuilder(); }
      
  //   ConfigBuilder& withPort(int port)           { config.setPort(port); return *this; }
  //   ConfigBuilder& withMaxBodySize(size_t size) { config.setTimeoutSeconds(size); return *this; }
  //   ConfigBuilder& withTimeout(int seconds)     { config.setMaxBodySize(size); return *this; }
      
  //   Config build() { return config; }
      
  //   private:
  //   Config config;
  // };
}
