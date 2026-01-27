#pragma once

#include <string_view>

namespace HTTP_METHOD {
    constexpr const char* GET = "GET";
    constexpr const char* POST = "POST";
    constexpr const char* PUT = "PUT";
    constexpr const char* DELETE = "DELETE";
    constexpr const char* PATCH = "PATCH";
    constexpr const char* OPTIONS = "OPTIONS";
    constexpr const char* HEAD = "HEAD";
}

namespace HTTP_STATUS {
    // 1xx Informational
    constexpr short CONTINUE = 100;
    constexpr short SWITCHING_PROTOCOLS = 101;
    constexpr short PROCESSING = 102;
    constexpr short EARLY_HINTS = 103;

    // 2xx Success
    constexpr short OK = 200;
    constexpr short CREATED = 201;
    constexpr short ACCEPTED = 202;
    constexpr short NON_AUTHORITATIVE_INFORMATION = 203;
    constexpr short NO_CONTENT = 204;
    constexpr short RESET_CONTENT = 205;
    constexpr short PARTIAL_CONTENT = 206;
    constexpr short MULTI_STATUS = 207;
    constexpr short ALREADY_REPORTED = 208;
    constexpr short IM_USED = 226;

    // 3xx Redirection
    constexpr short MULTIPLE_CHOICES = 300;
    constexpr short MOVED_PERMANENTLY = 301;
    constexpr short FOUND = 302;
    constexpr short SEE_OTHER = 303;
    constexpr short NOT_MODIFIED = 304;
    constexpr short USE_PROXY = 305;
    constexpr short TEMPORARY_REDIRECT = 307;
    constexpr short PERMANENT_REDIRECT = 308;

    // 4xx Client Errors
    constexpr short BAD_REQUEST = 400;
    constexpr short UNAUTHORIZED = 401;
    constexpr short PAYMENT_REQUIRED = 402;
    constexpr short FORBIDDEN = 403;
    constexpr short NOT_FOUND = 404;
    constexpr short METHOD_NOT_ALLOWED = 405;
    constexpr short NOT_ACCEPTABLE = 406;
    constexpr short PROXY_AUTHENTICATION_REQUIRED = 407;
    constexpr short REQUEST_TIMEOUT = 408;
    constexpr short CONFLICT = 409;
    constexpr short GONE = 410;
    constexpr short LENGTH_REQUIRED = 411;
    constexpr short PRECONDITION_FAILED = 412;
    constexpr short PAYLOAD_TOO_LARGE = 413;
    constexpr short URI_TOO_LONG = 414;
    constexpr short UNSUPPORTED_MEDIA_TYPE = 415;
    constexpr short RANGE_NOT_SATISFIABLE = 416;
    constexpr short EXPECTATION_FAILED = 417;
    constexpr short IM_A_TEAPOT = 418;
    constexpr short MISDIRECTED_REQUEST = 421;
    constexpr short UNPROCESSABLE_ENTITY = 422;
    constexpr short LOCKED = 423;
    constexpr short FAILED_DEPENDENCY = 424;
    constexpr short TOO_EARLY = 425;
    constexpr short UPGRADE_REQUIRED = 426;
    constexpr short PRECONDITION_REQUIRED = 428;
    constexpr short TOO_MANY_REQUESTS = 429;
    constexpr short REQUEST_HEADER_FIELDS_TOO_LARGE = 431;
    constexpr short UNAVAILABLE_FOR_LEGAL_REASONS = 451;

    // 5xx Server Errors
    constexpr short INTERNAL_SERVER_ERROR = 500;
    constexpr short NOT_IMPLEMENTED = 501;
    constexpr short BAD_GATEWAY = 502;
    constexpr short SERVICE_UNAVAILABLE = 503;
    constexpr short GATEWAY_TIMEOUT = 504;
    constexpr short HTTP_VERSION_NOT_SUPPORTED = 505;
    constexpr short VARIANT_ALSO_NEGOTIATES = 506;
    constexpr short INSUFFICIENT_STORAGE = 507;
    constexpr short LOOP_DETECTED = 508;
    constexpr short NOT_EXTENDED = 510;
    constexpr short NETWORK_AUTHENTICATION_REQUIRED = 511;
}

namespace HTTP_HEADER {
    inline constexpr std::string_view ACCEPT                  = "accept";
    inline constexpr std::string_view ACCEPT_CHARSET          = "accept-charset";
    inline constexpr std::string_view ACCEPT_ENCODING         = "accept-encoding";
    inline constexpr std::string_view ACCEPT_LANGUAGE         = "accept-language";
    inline constexpr std::string_view AUTHORIZATION           = "authorization";
    inline constexpr std::string_view CACHE_CONTROL           = "cache-control";
    inline constexpr std::string_view CONTENT_DISPOSITION     = "content-disposition";
    inline constexpr std::string_view CONTENT_ENCODING        = "content-encoding";
    inline constexpr std::string_view CONTENT_LANGUAGE        = "content-language";
    inline constexpr std::string_view CONTENT_LOCATION        = "content-location";
    inline constexpr std::string_view CONTENT_RANGE           = "content-range";
    inline constexpr std::string_view CONTENT_TYPE            = "content-type";
    inline constexpr std::string_view CONTENT_LENGTH          = "content-length";
    inline constexpr std::string_view COOKIE                  = "cookie";
    inline constexpr std::string_view CONNECTION              = "connection";
    inline constexpr std::string_view DATE                    = "date";
    inline constexpr std::string_view ETAG                    = "etag";
    inline constexpr std::string_view EXPECT                  = "expect";
    inline constexpr std::string_view EXPIRES                 = "expires";
    inline constexpr std::string_view FROM                    = "from";
    inline constexpr std::string_view HOST                    = "host";
    inline constexpr std::string_view IF_MATCH                = "if-match";
    inline constexpr std::string_view IF_MODIFIED_SINCE       = "if-modified-since";
    inline constexpr std::string_view IF_NONE_MATCH           = "if-none-match";
    inline constexpr std::string_view IF_RANGE                = "if-range";
    inline constexpr std::string_view IF_UNMODIFIED_SINCE     = "if-unmodified-since";
    inline constexpr std::string_view LAST_MODIFIED           = "last-modified";
    inline constexpr std::string_view LOCATION                = "location";
    inline constexpr std::string_view MAX_FORWARDS            = "max-forwards";
    inline constexpr std::string_view ORIGIN                  = "origin";
    inline constexpr std::string_view PRAGMA                  = "pragma";
    inline constexpr std::string_view PROXY_AUTHENTICATE      = "proxy-authenticate";
    inline constexpr std::string_view PROXY_AUTHORIZATION     = "proxy-authorization";
    inline constexpr std::string_view RANGE                   = "range";
    inline constexpr std::string_view REFERER                 = "referer";
    inline constexpr std::string_view RETRY_AFTER             = "retry-after";
    inline constexpr std::string_view SERVER                  = "server";
    inline constexpr std::string_view SET_COOKIE              = "set-cookie";
    inline constexpr std::string_view TE                      = "te";
    inline constexpr std::string_view TRAILER                 = "trailer";
    inline constexpr std::string_view TRANSFER_ENCODING       = "transfer-encoding";
    inline constexpr std::string_view UPGRADE                 = "upgrade";
    inline constexpr std::string_view USER_AGENT              = "user-agent";
    inline constexpr std::string_view VARY                    = "vary";
    inline constexpr std::string_view VIA                     = "via";
    inline constexpr std::string_view WARNING                 = "warning";
    inline constexpr std::string_view WWW_AUTHENTICATE        = "www-authenticate";
    inline constexpr std::string_view X_FORWARDED_FOR         = "x-forwarded-for";
    inline constexpr std::string_view X_FORWARDED_HOST        = "x-forwarded-host";
    inline constexpr std::string_view X_FORWARDED_PROTO       = "x-forwarded-proto";
    inline constexpr std::string_view X_REQUEST_ID            = "x-request-id";
    inline constexpr std::string_view X_CORRELATION_ID        = "x-correlation-id";
    inline constexpr std::string_view X_FRAME_OPTIONS         = "x-frame-options";
    inline constexpr std::string_view X_CONTENT_TYPE_OPTIONS  = "x-content-type-options";
    inline constexpr std::string_view X_XSS_PROTECTION        = "x-xss-protection";
    inline constexpr std::string_view X_RATE_LIMIT_LIMIT      = "x-ratelimit-limit";
    inline constexpr std::string_view X_RATE_LIMIT_REMAINING  = "x-ratelimit-remaining";
    inline constexpr std::string_view X_RATE_LIMIT_RESET      = "x-ratelimit-reset";
}

namespace HTTP_CONNECTION {
    inline constexpr std::string_view CLOSE          = "close";
    inline constexpr std::string_view KEEP_ALIVE     = "keep-alive";
    inline constexpr std::string_view UPGRADE        = "upgrade";
}

namespace HTTP_CONTENT_TYPE {
    inline constexpr std::string_view APPLICATION_JSON              = "application/json";
    inline constexpr std::string_view APPLICATION_XML               = "application/xml";
    inline constexpr std::string_view APPLICATION_FORM_URLENCODED   = "application/x-www-form-urlencoded";
    inline constexpr std::string_view MULTIPART_FORM_DATA           = "multipart/form-data";
    inline constexpr std::string_view TEXT_PLAIN                    = "text/plain";
    inline constexpr std::string_view TEXT_HTML                     = "text/html";
    inline constexpr std::string_view TEXT_CSS                      = "text/css";
    inline constexpr std::string_view TEXT_CSV                      = "text/csv";
    inline constexpr std::string_view APPLICATION_JAVASCRIPT        = "application/javascript";
    inline constexpr std::string_view APPLICATION_PDF               = "application/pdf";
    inline constexpr std::string_view IMAGE_PNG                     = "image/png";
    inline constexpr std::string_view IMAGE_JPEG                    = "image/jpeg";
    inline constexpr std::string_view IMAGE_GIF                     = "image/gif";
    inline constexpr std::string_view IMAGE_SVG_XML                 = "image/svg+xml";
    inline constexpr std::string_view APPLICATION_OCTET_STREAM      = "application/octet-stream";
}
