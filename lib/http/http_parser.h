#pragma once

#include <sstream>
#include <stdexcept>
#include <unistd.h>

#include "../context.h"
#include "../config.h"  
#include "../helpers.h"
#include "../constants.h"

namespace Metro {

class HttpParser {    
public:
    static inline bool parse(
        int clientSocket,
        Context& context,
        bool& keepAlive,
        const Config& config
    ) {
        std::string buffer;
        size_t total_bytes_read = 0;
        
        if (!readHeaders(clientSocket, buffer, total_bytes_read, config))
            return false;
        
        std::istringstream input(buffer);
        if (!parseRequestLine(input, context, config))
            return false;
        
        if (!parseHeaders(input, context, config))
            return false;
        
        keepAlive = shouldKeepAlive(context.req);
        
        // Check content-length against max body size
        auto lenHeader = context.req.header(
            std::string(Constants::Http_Header::CONTENT_LENGTH));
        
        if (lenHeader) {
            size_t content_length = std::stoul(*lenHeader);
            if (content_length > config.security().max_body_size) {
                context.req.setParsingError("Request body too large");
                context.res.status(Constants::Http_Status::PAYLOAD_TOO_LARGE)
                          .text("Payload Too Large");
                return false;
            }
        }
        
        // Check total request size (headers + body)
        if (total_bytes_read > config.security().max_body_size * 2) {
            context.req.setParsingError("Request too large");
            context.res.status(Constants::Http_Status::PAYLOAD_TOO_LARGE)
                      .text("Request Too Large");
            return false;
        }
        
        if (!parseBody(clientSocket, buffer, context, config))
            return false;
        
        return true;
    }
    
private:
    static inline bool readHeaders(
        int clientSocket,
        std::string& buffer,
        size_t& total_bytes_read,
        const Config& config
    ) {
        char chunk[config.server().buffer_size];
        size_t max_size = config.security().max_body_size * 2;
        
        while (buffer.find("\r\n\r\n") == std::string::npos) {
            ssize_t n = recv(clientSocket, chunk, sizeof(chunk), 0);
            if (n <= 0) return false;
            
            buffer.append(chunk, n);
            total_bytes_read += n;
            
            // Check if we're exceeding limits
            if (buffer.size() > config.server().max_header_size) {
                return false;  // Headers too large
            }
            
            if (total_bytes_read > max_size) {
                return false;  // Total request too large
            }
        }
        return true;
    }
    
    static inline bool parseRequestLine(
        std::istream& in,
        Context& context,
        const Config& config
    ) {
        std::string rawPath, version;
        in >> context.req._method >> rawPath >> version;
        
        // Store original path
        context.req._original_path = rawPath;
        
        // Apply path sanitization if enabled
        if (config.security().enable_path_sanitization) {
            rawPath = sanitizePath(rawPath, config);
            if (rawPath.empty()) {
                context.req.setParsingError("Invalid path");
                context.res.status(Constants::Http_Status::BAD_REQUEST)
                          .text("Bad Request: Invalid path");
                return false;
            }
        }
        
        auto q = rawPath.find('?');
        if (q != std::string::npos) {
            context.req._path = rawPath.substr(0, q);
            // Limit query parameters count
            if (countQueryParams(rawPath.substr(q + 1)) > config.security().max_query_params) {
                context.req.setParsingError("Too many query parameters");
                context.res.status(Constants::Http_Status::URI_TOO_LONG)
                          .text("Bad Request: Too many query parameters");
                return false;
            }
            Helpers::parseQuery(
                rawPath.substr(q + 1),
                context.req._query
            );
        } else {
            context.req._path = rawPath;
        }
        
        return true;
    }
    
    static inline bool parseHeaders(
        std::istream& in,
        Context& context,
        const Config& config
    ) {
        std::string line;
        std::getline(in, line);
        
        size_t header_count = 0;
        
        while (std::getline(in, line) && line != "\r") {
            if (++header_count > config.security().max_headers_count) {
                context.req.setParsingError("Too many headers");
                context.res.status(Constants::Http_Status::REQUEST_HEADER_FIELDS_TOO_LARGE)
                          .text("Too Many Headers");
                return false;
            }
            
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;
            
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            Helpers::trim(val);
            
            // Check for header size limits
            if (key.size() + val.size() > config.server().buffer_size) {  // Per-header limit
                context.req.setParsingError("Header too large");
                context.res.status(Constants::Http_Status::REQUEST_HEADER_FIELDS_TOO_LARGE)
                          .text("Header Too Large");
                return false;
            }
            
            context.req._headers[key] = val;
        }
        return true;
    }
    
    static inline bool parseBody(
        int clientSocket,
        const std::string& buffer,
        Context& context,
        const Config& config
    ) {
        auto transferEncoding = context.req.header(std::string(Constants::Http_Header::TRANSFER_ENCODING));
        
        if (transferEncoding && *transferEncoding == "chunked") {
            if (!config.features().enable_chunked_encoding) {
                context.req.setParsingError("Chunked encoding not supported");
                context.res.status(Constants::Http_Status::LENGTH_REQUIRED)
                          .text("Chunked Encoding Not Supported");
                return false;
            }
            return parseChunkedBody(clientSocket, buffer, context, config);
        } else {
            return parseContentLengthBody(clientSocket, buffer, context, config);
        }
    }
    
    static inline bool parseContentLengthBody(
        int clientSocket,
        const std::string& buffer,
        Context& context,
        const Config& config
    ) {
        auto lenHeader = context.req.header(
            std::string(Constants::Http_Header::CONTENT_LENGTH));
        
        if (!lenHeader) {
            context.req._body.clear();
            return true;  // No body
        }
        
        size_t len = std::stoul(*lenHeader);
        
        // Double-check size limit
        if (len > config.security().max_body_size) {
            context.req.setParsingError("Request body too large");
            context.res.status(Constants::Http_Status::PAYLOAD_TOO_LARGE)
                      .text("Payload Too Large");
            return false;
        }
        
        size_t headerEnd = buffer.find("\r\n\r\n") + 4;
        context.req._body.assign(
            buffer.data() + headerEnd,
            std::min(len, buffer.size() - headerEnd)
        );
        
        while (context.req._body.size() < len) {
            char chunk[config.server().buffer_size];
            ssize_t remaining = len - context.req._body.size();
            ssize_t to_read = std::min(sizeof(chunk), static_cast<size_t>(remaining));
            
            ssize_t n = recv(clientSocket, chunk, to_read, 0);
            if (n <= 0) break;
            
            context.req._body.append(chunk, n);
            
            // Additional progress check against limit
            if (context.req._body.size() > config.security().max_body_size) {
                context.req.setParsingError("Request body exceeds limit during streaming");
                context.res.status(Constants::Http_Status::PAYLOAD_TOO_LARGE)
                          .text("Payload Too Large");
                return false;
            }
        }
        
        return true;
    }
    
    // Helper functions for new features
    static inline std::string sanitizePath(
        const std::string& path,
        const Config& config
    ) {
        std::string sanitized = path;
        
        // Remove null bytes
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\0'), sanitized.end());
        
        // Decode URL encoding first
        sanitized = Helpers::urlDecode(sanitized);
        
        // If double encoding check is enabled, decode again and compare
        if (config.security().reject_double_encoded_paths) {
            std::string doubleDecoded = Helpers::urlDecode(sanitized);
            if (doubleDecoded != sanitized && doubleDecoded.find("..") != std::string::npos) {
                return "";  // Likely double-encoded path traversal attempt
            }
        }
        
        // Check for path traversal attempts
        if (sanitized.find("..") != std::string::npos) {
            return "";  // Path traversal attempt detected
        }
        
        // Remove directory traversal patterns
        size_t pos;
        while ((pos = sanitized.find("/../")) != std::string::npos) {
            return "";  // Invalid path
        }
        
        while ((pos = sanitized.find("/./")) != std::string::npos) {
            sanitized.erase(pos, 2);
        }
        
        // Remove trailing slash except for root
        if (sanitized.size() > 1 && sanitized.back() == '/') {
            sanitized.pop_back();
        }
        
        // Ensure path starts with /
        if (sanitized.empty() || sanitized[0] != '/') {
            sanitized = "/" + sanitized;
        }
        
        return sanitized;
    }
    
    static inline size_t countQueryParams(const std::string& queryString) {
        if (queryString.empty()) return 0;
        return std::count(queryString.begin(), queryString.end(), '&') + 1;
    }
    
    static bool parseChunkedBody(
        int clientSocket,
        const std::string& buffer,
        Context& context,
        const Config& config
    ) {
        size_t headerEnd = buffer.find("\r\n\r\n") + 4;
        std::string data(buffer.begin() + headerEnd, buffer.end());
        
        std::string body;
        body.reserve(config.security().max_body_size);
        
        size_t totalSize = 0;
        size_t chunkSize = 0;
        
        while (true) {
            // Find chunk size line
            size_t lineEnd = data.find("\r\n");
            if (lineEnd == std::string::npos) {
                // Need more data
                if (!readMoreData(clientSocket, data, config)) {
                    context.req.setParsingError("Invalid chunked encoding");
                    return false;
                }
                continue;
            }
            
            std::string sizeLine = data.substr(0, lineEnd);
            
            // Parse chunk size (hex)
            try {
                chunkSize = std::stoul(sizeLine, nullptr, 16);
            } catch (const std::exception&) {
                context.req.setParsingError("Invalid chunk size");
                return false;
            }
            
            // Last chunk
            if (chunkSize == 0) {
                // Skip trailing "\r\n"
                data.erase(0, lineEnd + 2);
                
                // Check for trailers
                if (data.size() >= 2 && data.substr(0, 2) == "\r\n") {
                    data.erase(0, 2);
                    parseTrailers(data, context);
                }
                
                context.req._body = std::move(body);
                return true;
            }
            
            // Remove size line
            data.erase(0, lineEnd + 2);
            
            // Check if we have enough data for the chunk
            while (data.size() < chunkSize + 2) {  // +2 for "\r\n" after chunk
                if (!readMoreData(clientSocket, data, config)) {
                    context.req.setParsingError("Incomplete chunked data");
                    return false;
                }
            }
            
            // Extract chunk data
            std::string chunk = data.substr(0, chunkSize);
            body.append(chunk);
            totalSize += chunkSize;
            
            // Check size limit
            if (totalSize > config.security().max_body_size) {
                context.req.setParsingError("Chunked body exceeds size limit");
                context.res.status(Constants::Http_Status::PAYLOAD_TOO_LARGE)
                          .text("Payload Too Large");
                return false;
            }
            
            // Remove chunk data and trailing "\r\n"
            data.erase(0, chunkSize + 2);
        }
    }

    static bool readMoreData(
        int clientSocket,
        std::string& buffer,
        const Config& config
    ) {
        char chunk[config.server().buffer_size];
        ssize_t n = recv(clientSocket, chunk, sizeof(chunk), 0);
        
        if (n <= 0) return false;
        
        buffer.append(chunk, n);
        
        // Check buffer size limit
        if (buffer.size() > config.security().max_body_size * 2) {
            return false;
        }
        
        return true;
    }

    static void parseTrailers(
        const std::string& data,
        Context& context
    ) {
        std::istringstream stream(data);
        std::string line;
        
        while (std::getline(stream, line) && line != "\r") {
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;
            
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            Helpers::trim(val);
            
            context.req._headers["X-Trailer-" + key] = val;
        }
    }

    static inline bool shouldKeepAlive(const Request& req) {
        auto conn = req.header(std::string(Constants::Http_Header::CONNECTION));
        if (!conn) return true;
        return *conn != Constants::Http_Connection::CLOSE;
    }
};
}
