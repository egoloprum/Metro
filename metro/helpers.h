#pragma once

#include <chrono>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#include "constants.h"

namespace Metro {
  namespace Helpers {
    inline const char* reasonPhrase(int statusCode) {
      switch (statusCode) {
        // 1xx Informational
        case Constants::Http_Status::CONTINUE: return "Continue";
        case Constants::Http_Status::SWITCHING_PROTOCOLS: return "Switching Protocols";
        case Constants::Http_Status::PROCESSING: return "Processing";
        case Constants::Http_Status::EARLY_HINTS: return "Early Hints";
        
        // 2xx Success
        case Constants::Http_Status::OK: return "OK";
        case Constants::Http_Status::CREATED: return "Created";
        case Constants::Http_Status::ACCEPTED: return "Accepted";
        case Constants::Http_Status::NON_AUTHORITATIVE_INFORMATION: return "Non-Authoritative Information";
        case Constants::Http_Status::NO_CONTENT: return "No Content";
        case Constants::Http_Status::RESET_CONTENT: return "Reset Content";
        case Constants::Http_Status::PARTIAL_CONTENT: return "Partial Content";
        case Constants::Http_Status::MULTI_STATUS: return "Multi-Status";
        case Constants::Http_Status::ALREADY_REPORTED: return "Already Reported";
        case Constants::Http_Status::IM_USED: return "IM Used";
        
        // 3xx Redirection
        case Constants::Http_Status::MULTIPLE_CHOICES: return "Multiple Choices";
        case Constants::Http_Status::MOVED_PERMANENTLY: return "Moved Permanently";
        case Constants::Http_Status::FOUND: return "Found";
        case Constants::Http_Status::SEE_OTHER: return "See Other";
        case Constants::Http_Status::NOT_MODIFIED: return "Not Modified";
        case Constants::Http_Status::USE_PROXY: return "Use Proxy";
        case Constants::Http_Status::TEMPORARY_REDIRECT: return "Temporary Redirect";
        case Constants::Http_Status::PERMANENT_REDIRECT: return "Permanent Redirect";
        
        // 4xx Client Errors
        case Constants::Http_Status::BAD_REQUEST: return "Bad Request";
        case Constants::Http_Status::UNAUTHORIZED: return "Unauthorized";
        case Constants::Http_Status::PAYMENT_REQUIRED: return "Payment Required";
        case Constants::Http_Status::FORBIDDEN: return "Forbidden";
        case Constants::Http_Status::NOT_FOUND: return "Not Found";
        case Constants::Http_Status::METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case Constants::Http_Status::NOT_ACCEPTABLE: return "Not Acceptable";
        case Constants::Http_Status::PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
        case Constants::Http_Status::REQUEST_TIMEOUT: return "Request Timeout";
        case Constants::Http_Status::CONFLICT: return "Conflict";
        case Constants::Http_Status::GONE: return "Gone";
        case Constants::Http_Status::LENGTH_REQUIRED: return "Length Required";
        case Constants::Http_Status::PRECONDITION_FAILED: return "Precondition Failed";
        case Constants::Http_Status::PAYLOAD_TOO_LARGE: return "Payload Too Large";
        case Constants::Http_Status::URI_TOO_LONG: return "URI Too Long";
        case Constants::Http_Status::UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
        case Constants::Http_Status::RANGE_NOT_SATISFIABLE: return "Range Not Satisfiable";
        case Constants::Http_Status::EXPECTATION_FAILED: return "Expectation Failed";
        case Constants::Http_Status::IM_A_TEAPOT: return "I'm a teapot";
        case Constants::Http_Status::MISDIRECTED_REQUEST: return "Misdirected Request";
        case Constants::Http_Status::UNPROCESSABLE_ENTITY: return "Unprocessable Entity";
        case Constants::Http_Status::LOCKED: return "Locked";
        case Constants::Http_Status::FAILED_DEPENDENCY: return "Failed Dependency";
        case Constants::Http_Status::TOO_EARLY: return "Too Early";
        case Constants::Http_Status::UPGRADE_REQUIRED: return "Upgrade Required";
        case Constants::Http_Status::PRECONDITION_REQUIRED: return "Precondition Required";
        case Constants::Http_Status::TOO_MANY_REQUESTS: return "Too Many Requests";
        case Constants::Http_Status::REQUEST_HEADER_FIELDS_TOO_LARGE: return "Request Header Fields Too Large";
        case Constants::Http_Status::UNAVAILABLE_FOR_LEGAL_REASONS: return "Unavailable For Legal Reasons";
        
        // 5xx Server Errors
        case Constants::Http_Status::INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case Constants::Http_Status::NOT_IMPLEMENTED: return "Not Implemented";
        case Constants::Http_Status::BAD_GATEWAY: return "Bad Gateway";
        case Constants::Http_Status::SERVICE_UNAVAILABLE: return "Service Unavailable";
        case Constants::Http_Status::GATEWAY_TIMEOUT: return "Gateway Timeout";
        case Constants::Http_Status::HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
        case Constants::Http_Status::VARIANT_ALSO_NEGOTIATES: return "Variant Also Negotiates";
        case Constants::Http_Status::INSUFFICIENT_STORAGE: return "Insufficient Storage";
        case Constants::Http_Status::LOOP_DETECTED: return "Loop Detected";
        case Constants::Http_Status::NOT_EXTENDED: return "Not Extended";
        case Constants::Http_Status::NETWORK_AUTHENTICATION_REQUIRED: return "Network Authentication Required";
        
        default: return "Unknown";
      }
    }

    class PathSanitizer {
      public:

      static std::string normalize(const std::string& path) {
        // Step 1: Split into segments and decode each segment individually
        std::vector<std::string> segments;
        std::string current;
        
        for (size_t i = 0; i < path.size(); ++i) {
          char c = path[i];
          
          if (c == '/') {
            if (!current.empty()) {
              segments.push_back(decodeSegment(current));
              current.clear();
            }
            continue;
          } else {
            current += c;
          }
        }
        if (!current.empty()) {
          segments.push_back(decodeSegment(current));
        }

        // Step 2: Resolve path traversal using stack
        std::vector<std::string> resolved;
        for (const auto& seg : segments) {
          if (seg == "." || seg.empty()) {
            continue; 
          } else if (seg == "..") {
            if (!resolved.empty()) {
              resolved.pop_back();  
            }
          } else {
            resolved.push_back(seg);
          }
        }

        // Step 3: Reconstruct path
        std::string result = "/";
        for (size_t i = 0; i < resolved.size(); ++i) {
          if (i > 0) result += "/";
          result += resolved[i];
        }
        
        return result;
      }

      static std::string encodeSegment(const std::string& input, bool formData = false) {
        std::string encoded;
        encoded.reserve(input.size() * 3);
        
        for (unsigned char ch : input) {
          if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            encoded += ch;
          } 
          else if (ch == ' ' && formData) {
            encoded += '+';
          } 
          else {
            const char* hexDigits = "0123456789ABCDEF";
            encoded += '%';
            encoded += hexDigits[ch >> 4];      
            encoded += hexDigits[ch & 0x0F];    
          }
        }
        
        return encoded;
      }

      static std::string decodeSegment(const std::string& encoded) {
        std::string decoded;
        decoded.reserve(encoded.size());
        
        for (size_t i = 0; i < encoded.size(); ++i) {
          if (encoded[i] == '%' && i + 2 < encoded.size()) {
            char hi = encoded[i + 1];
            char lo = encoded[i + 2];
            
            if (std::isxdigit(hi) && std::isxdigit(lo)) {
              unsigned char val = (hexValue(hi) << 4) | hexValue(lo);
              
              // Security: Reject null bytes and path separators
              if (val == '\0' || val == '/' || val == '\\') {
                return "";  // Invalid segment
              }
              
              decoded += static_cast<char>(val);
              i += 2;
            } else {
              // Invalid encoding, keep literal %
              decoded += '%';
            }
          } else if (encoded[i] == '\0' || encoded[i] == '/' || encoded[i] == '\\') {
            // Reject embedded nulls or separators
            return "";
          } else {
            decoded += encoded[i];
          }
        }
        return decoded;
      }

      private:
      
      static unsigned char hexValue(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
      }
    };
  }
}  

