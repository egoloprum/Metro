#pragma once

#include <chrono>
#include <sstream>
#include <iomanip>

#include "constants.h"

namespace METRO_HELPERS {
    inline const char* reasonPhrase(int statusCode) {
        switch (statusCode) {
            // 1xx Informational
            case HTTP_STATUS::CONTINUE: return "Continue";
            case HTTP_STATUS::SWITCHING_PROTOCOLS: return "Switching Protocols";
            case HTTP_STATUS::PROCESSING: return "Processing";
            case HTTP_STATUS::EARLY_HINTS: return "Early Hints";
            
            // 2xx Success
            case HTTP_STATUS::OK: return "OK";
            case HTTP_STATUS::CREATED: return "Created";
            case HTTP_STATUS::ACCEPTED: return "Accepted";
            case HTTP_STATUS::NON_AUTHORITATIVE_INFORMATION: return "Non-Authoritative Information";
            case HTTP_STATUS::NO_CONTENT: return "No Content";
            case HTTP_STATUS::RESET_CONTENT: return "Reset Content";
            case HTTP_STATUS::PARTIAL_CONTENT: return "Partial Content";
            case HTTP_STATUS::MULTI_STATUS: return "Multi-Status";
            case HTTP_STATUS::ALREADY_REPORTED: return "Already Reported";
            case HTTP_STATUS::IM_USED: return "IM Used";
            
            // 3xx Redirection
            case HTTP_STATUS::MULTIPLE_CHOICES: return "Multiple Choices";
            case HTTP_STATUS::MOVED_PERMANENTLY: return "Moved Permanently";
            case HTTP_STATUS::FOUND: return "Found";
            case HTTP_STATUS::SEE_OTHER: return "See Other";
            case HTTP_STATUS::NOT_MODIFIED: return "Not Modified";
            case HTTP_STATUS::USE_PROXY: return "Use Proxy";
            case HTTP_STATUS::TEMPORARY_REDIRECT: return "Temporary Redirect";
            case HTTP_STATUS::PERMANENT_REDIRECT: return "Permanent Redirect";
            
            // 4xx Client Errors
            case HTTP_STATUS::BAD_REQUEST: return "Bad Request";
            case HTTP_STATUS::UNAUTHORIZED: return "Unauthorized";
            case HTTP_STATUS::PAYMENT_REQUIRED: return "Payment Required";
            case HTTP_STATUS::FORBIDDEN: return "Forbidden";
            case HTTP_STATUS::NOT_FOUND: return "Not Found";
            case HTTP_STATUS::METHOD_NOT_ALLOWED: return "Method Not Allowed";
            case HTTP_STATUS::NOT_ACCEPTABLE: return "Not Acceptable";
            case HTTP_STATUS::PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
            case HTTP_STATUS::REQUEST_TIMEOUT: return "Request Timeout";
            case HTTP_STATUS::CONFLICT: return "Conflict";
            case HTTP_STATUS::GONE: return "Gone";
            case HTTP_STATUS::LENGTH_REQUIRED: return "Length Required";
            case HTTP_STATUS::PRECONDITION_FAILED: return "Precondition Failed";
            case HTTP_STATUS::PAYLOAD_TOO_LARGE: return "Payload Too Large";
            case HTTP_STATUS::URI_TOO_LONG: return "URI Too Long";
            case HTTP_STATUS::UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
            case HTTP_STATUS::RANGE_NOT_SATISFIABLE: return "Range Not Satisfiable";
            case HTTP_STATUS::EXPECTATION_FAILED: return "Expectation Failed";
            case HTTP_STATUS::IM_A_TEAPOT: return "I'm a teapot";
            case HTTP_STATUS::MISDIRECTED_REQUEST: return "Misdirected Request";
            case HTTP_STATUS::UNPROCESSABLE_ENTITY: return "Unprocessable Entity";
            case HTTP_STATUS::LOCKED: return "Locked";
            case HTTP_STATUS::FAILED_DEPENDENCY: return "Failed Dependency";
            case HTTP_STATUS::TOO_EARLY: return "Too Early";
            case HTTP_STATUS::UPGRADE_REQUIRED: return "Upgrade Required";
            case HTTP_STATUS::PRECONDITION_REQUIRED: return "Precondition Required";
            case HTTP_STATUS::TOO_MANY_REQUESTS: return "Too Many Requests";
            case HTTP_STATUS::REQUEST_HEADER_FIELDS_TOO_LARGE: return "Request Header Fields Too Large";
            case HTTP_STATUS::UNAVAILABLE_FOR_LEGAL_REASONS: return "Unavailable For Legal Reasons";
            
            // 5xx Server Errors
            case HTTP_STATUS::INTERNAL_SERVER_ERROR: return "Internal Server Error";
            case HTTP_STATUS::NOT_IMPLEMENTED: return "Not Implemented";
            case HTTP_STATUS::BAD_GATEWAY: return "Bad Gateway";
            case HTTP_STATUS::SERVICE_UNAVAILABLE: return "Service Unavailable";
            case HTTP_STATUS::GATEWAY_TIMEOUT: return "Gateway Timeout";
            case HTTP_STATUS::HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
            case HTTP_STATUS::VARIANT_ALSO_NEGOTIATES: return "Variant Also Negotiates";
            case HTTP_STATUS::INSUFFICIENT_STORAGE: return "Insufficient Storage";
            case HTTP_STATUS::LOOP_DETECTED: return "Loop Detected";
            case HTTP_STATUS::NOT_EXTENDED: return "Not Extended";
            case HTTP_STATUS::NETWORK_AUTHENTICATION_REQUIRED: return "Network Authentication Required";
            
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
}


