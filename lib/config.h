#pragma once

#include <string>
#include <optional>
#include <variant>
#include <unordered_map>

namespace Metro {

class Config {
public:
    // Server Configuration
    struct ServerConfig {
        int port = 3000;
        int timeout_seconds = 5;
        int backlog_size = 128;
        size_t buffer_size = 8192;
        size_t max_header_size = 64 * 1024;
    };

    // Security Configuration
    struct SecurityConfig {
        size_t max_body_size = 10 * 1024 * 1024; // 10MB default
        bool enable_path_sanitization = true;
        bool reject_double_encoded_paths = true;
        size_t max_query_params = 100;
        size_t max_headers_count = 100;
    };

    // Feature Flags
    struct FeatureConfig {
        bool enable_chunked_encoding = true;
        bool enable_gzip_compression = false;
        bool enable_request_logging = true;
        bool enable_keep_alive = true;
    };

    // Static configuration loading
    static Config fromYaml(const std::string& yamlPath);
    static Config fromJson(const std::string& jsonPath);
    static Config fromEnv();

    // Getters
    const ServerConfig& server() const { return server_config; }
    const SecurityConfig& security() const { return security_config; }
    const FeatureConfig& features() const { return feature_config; }

    // Setters (Fluent API)
    Config& setPort(int port) { server_config.port = port; return *this; }
    Config& setMaxBodySize(size_t size) { security_config.max_body_size = size; return *this; }
    Config& enableChunkedEncoding(bool enable = true) { 
        feature_config.enable_chunked_encoding = enable; 
        return *this; 
    }
    Config& enablePathSanitization(bool enable = true) { 
        security_config.enable_path_sanitization = enable; 
        return *this; 
    }

private:
    ServerConfig server_config;
    SecurityConfig security_config;
    FeatureConfig feature_config;
};

// Configuration builder for easy setup
class ConfigBuilder {
public:
    static ConfigBuilder create() { return ConfigBuilder(); }
    
    ConfigBuilder& withPort(int port) { config.setPort(port); return *this; }
    ConfigBuilder& withMaxBodySize(size_t size) { config.setMaxBodySize(size); return *this; }
    ConfigBuilder& withTimeout(int seconds) { 
        // Need to add timeout setter to Config
        return *this; 
    }
    
    Config build() { return config; }
    
private:
    Config config;
};

} // namespace Metro
