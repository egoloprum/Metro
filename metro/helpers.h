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
      /**
       * Normalizes a URL path for safe filesystem access.
       * 
       * SECURITY NOTES:
       * - This function treats paths as byte sequences (raw octets)
       * - It does NOT validate UTF-8 or perform Unicode normalization
       * - Safe for Linux ext4/XFS (byte-oriented filesystems)
       * - CAUTION: macOS (APFS/HFS+) and Windows perform Unicode normalization.
       *   On these platforms, different byte sequences may resolve to the same 
       *   filename (NFD vs NFC). Use strict mode or additional validation.
       * - Rejects overlong UTF-8 encodings of '.' and '/' via byte-value checking
      */

      static std::string normalize(const std::string& path, bool strictUtf8 = false) {
        // Step 1: Split into segments and decode each segment individually
        std::vector<std::string> segments;
        std::string current;
        
        for (size_t i = 0; i < path.size(); ++i) {
          char c = path[i];
          
          if (c == '/') {
            if (!current.empty()) {
              std::string decoded = decodeSegment(current);
              if (strictUtf8 && !isValidUtf8(decoded)) {
                return ""; // Invalid UTF-8 in strict mode
              }
              segments.push_back(decoded);
              current.clear();
            }
            continue;
          } else {
            current += c;
          }
        }
        if (!current.empty()) {
          std::string decoded = decodeSegment(current);
          if (strictUtf8 && !isValidUtf8(decoded)) {
            return ""; // Invalid UTF-8 in strict mode
          }
          segments.push_back(decoded);
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

      static bool isValidUtf8(const std::string& str) {
        size_t i = 0;
        while (i < str.size()) {
          unsigned char c = str[i];
          
          // ASCII (0xxxxxxx)
          if ((c & 0x80) == 0) {
            ++i;
            continue;
          }
          
          // Check for overlong encodings and invalid sequences
          size_t bytes;
          unsigned char mask;
          
          if ((c & 0xE0) == 0xC0) {      // 110xxxxx - 2 bytes
            bytes = 2;
            mask = 0x1F;
            // Overlong check: C0 and C1 are invalid (would encode ASCII < 0x80)
            if (c == 0xC0 || c == 0xC1) return false;
          } else if ((c & 0xF0) == 0xE0) { // 1110xxxx - 3 bytes
            bytes = 3;
            mask = 0x0F;
          } else if ((c & 0xF8) == 0xF0) { // 11110xxx - 4 bytes
            bytes = 4;
            mask = 0x07;
            // Max valid Unicode is U+10FFFF (first byte <= 0xF4)
            if (c > 0xF4) return false;
          } else {
            // Invalid start byte (10xxxxxx or 11111xxx)
            return false;
          }
          
          // Check we have enough continuation bytes
          if (i + bytes > str.size()) return false;
          
          // Validate continuation bytes (10xxxxxx)
          unsigned int codepoint = c & mask;
          for (size_t j = 1; j < bytes; ++j) {
            unsigned char next = str[i + j];
            if ((next & 0xC0) != 0x80) return false; // Must be 10xxxxxx
            codepoint = (codepoint << 6) | (next & 0x3F);
          }
          
          // Check for overlong encodings (codepoint too small for byte length)
          if (bytes == 2 && codepoint < 0x80) return false;
          if (bytes == 3 && codepoint < 0x800) return false;
          if (bytes == 4 && codepoint < 0x10000) return false;
          
          // Check for surrogates (U+D800-U+DFFF) - invalid in UTF-8
          if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return false;
          
          i += bytes;
        }
        return true;
      }
    };
  }
}  

