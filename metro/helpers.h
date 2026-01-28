#pragma once

#include <chrono>
#include <sstream>
#include <iomanip>

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
  
      std::string getCurrentDate() {
          auto now = std::chrono::system_clock::now();
          std::time_t now_time = std::chrono::system_clock::to_time_t(now);
          
          std::tm gmt_time;
          gmtime_r(&now_time, &gmt_time);
          
          std::stringstream ss;
          ss << std::put_time(&gmt_time, "%a, %d %b %Y %H:%M:%S GMT");
          return ss.str();
      }
  
      inline std::string urlDecode(const std::string& s) {
          std::string out;
          out.reserve(s.size());
  
          for (size_t i = 0; i < s.size(); ++i) {
              if (s[i] == '+') {
                  out += ' ';
              } else if (s[i] == '%' && i + 2 < s.size()) {
                  char h1 = s[i + 1];
                  char h2 = s[i + 2];
  
                  if (std::isxdigit(h1) && std::isxdigit(h2)) {
                      int val = std::stoi(s.substr(i + 1, 2), nullptr, 16);
                      out += static_cast<char>(val);
                      i += 2;
                  } else {
                      out += s[i];
                  }
              } else {
                  out += s[i];
              }
          }
  
          return out;
      }
  
      inline void parseQuery(
          const std::string& qs,
          std::unordered_map<std::string, std::vector<std::string>>& out
      ) {
          std::istringstream ss(qs);
          std::string pair;
  
          while (std::getline(ss, pair, '&')) {
              if (pair.empty()) continue;
  
              auto eq = pair.find('=');
  
              std::string key = urlDecode(pair.substr(0, eq));
              std::string val = (eq == std::string::npos)
                  ? ""
                  : urlDecode(pair.substr(eq + 1));
  
              out[key].push_back(val);
          }
      }
  
      inline void ltrim(std::string& s) {
          size_t i = 0;
          while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n'))
              ++i;
          s.erase(0, i);
      }
  
      inline void rtrim(std::string& s) {
          size_t i = s.size();
          while (i > 0 && (s[i - 1] == ' ' || s[i - 1] == '\t' || s[i - 1] == '\r' || s[i - 1] == '\n'))
              --i;
          s.erase(i);
      }
  
      inline void trim(std::string& s) {
          ltrim(s);
          rtrim(s);
      }
  }
}  

