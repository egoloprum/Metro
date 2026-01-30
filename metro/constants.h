#pragma once

namespace Metro {
  namespace Constants {
    namespace Http_Method {
        constexpr const char* GET = "GET";
        constexpr const char* POST = "POST";
        constexpr const char* PUT = "PUT";
        constexpr const char* DELETE = "DELETE";
        constexpr const char* PATCH = "PATCH";
        constexpr const char* OPTIONS = "OPTIONS";
        constexpr const char* HEAD = "HEAD";
    }

    namespace Http_Status {
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
    
    namespace Http_Header {
        constexpr const char* ACCEPT                  = "accept";
        constexpr const char* ACCEPT_CHARSET          = "accept-charset";
        constexpr const char* ACCEPT_ENCODING         = "accept-encoding";
        constexpr const char* ACCEPT_LANGUAGE         = "accept-language";
        constexpr const char* ALLOW                   = "allow";
        constexpr const char* AUTHORIZATION           = "authorization";
        constexpr const char* CACHE_CONTROL           = "cache-control";
        constexpr const char* CONTENT_DISPOSITION     = "content-disposition";
        constexpr const char* CONTENT_ENCODING        = "content-encoding";
        constexpr const char* CONTENT_LANGUAGE        = "content-language";
        constexpr const char* CONTENT_LOCATION        = "content-location";
        constexpr const char* CONTENT_RANGE           = "content-range";
        constexpr const char* CONTENT_TYPE            = "content-type";
        constexpr const char* CONTENT_LENGTH          = "content-length";
        constexpr const char* COOKIE                  = "cookie";
        constexpr const char* CONNECTION              = "connection";
        constexpr const char* DATE                    = "date";
        constexpr const char* ETAG                    = "etag";
        constexpr const char* EXPECT                  = "expect";
        constexpr const char* EXPIRES                 = "expires";
        constexpr const char* FROM                    = "from";
        constexpr const char* HOST                    = "host";
        constexpr const char* IF_MATCH                = "if-match";
        constexpr const char* IF_MODIFIED_SINCE       = "if-modified-since";
        constexpr const char* IF_NONE_MATCH           = "if-none-match";
        constexpr const char* IF_RANGE                = "if-range";
        constexpr const char* IF_UNMODIFIED_SINCE     = "if-unmodified-since";
        constexpr const char* LAST_MODIFIED           = "last-modified";
        constexpr const char* LOCATION                = "location";
        constexpr const char* MAX_FORWARDS            = "max-forwards";
        constexpr const char* ORIGIN                  = "origin";
        constexpr const char* PRAGMA                  = "pragma";
        constexpr const char* PROXY_AUTHENTICATE      = "proxy-authenticate";
        constexpr const char* PROXY_AUTHORIZATION     = "proxy-authorization";
        constexpr const char* RANGE                   = "range";
        constexpr const char* REFERER                 = "referer";
        constexpr const char* RETRY_AFTER             = "retry-after";
        constexpr const char* SERVER                  = "server";
        constexpr const char* SET_COOKIE              = "set-cookie";
        constexpr const char* TE                      = "te";
        constexpr const char* TRAILER                 = "trailer";
        constexpr const char* TRANSFER_ENCODING       = "transfer-encoding";
        constexpr const char* UPGRADE                 = "upgrade";
        constexpr const char* USER_AGENT              = "user-agent";
        constexpr const char* VARY                    = "vary";
        constexpr const char* VIA                     = "via";
        constexpr const char* WARNING                 = "warning";
        constexpr const char* WWW_AUTHENTICATE        = "www-authenticate";
        constexpr const char* X_FORWARDED_FOR         = "x-forwarded-for";
        constexpr const char* X_FORWARDED_HOST        = "x-forwarded-host";
        constexpr const char* X_FORWARDED_PROTO       = "x-forwarded-proto";
        constexpr const char* X_REQUEST_ID            = "x-request-id";
        constexpr const char* X_CORRELATION_ID        = "x-correlation-id";
        constexpr const char* X_FRAME_OPTIONS         = "x-frame-options";
        constexpr const char* X_CONTENT_TYPE_OPTIONS  = "x-content-type-options";
        constexpr const char* X_XSS_PROTECTION        = "x-xss-protection";
        constexpr const char* X_RATE_LIMIT_LIMIT      = "x-ratelimit-limit";
        constexpr const char* X_RATE_LIMIT_REMAINING  = "x-ratelimit-remaining";
        constexpr const char* X_RATE_LIMIT_RESET      = "x-ratelimit-reset";
    }
    
    namespace Http_Connection {
        constexpr const char* CLOSE          = "close";
        constexpr const char* KEEP_ALIVE     = "keep-alive";
        constexpr const char* UPGRADE        = "upgrade";
    }
    
    namespace Http_Content_Type {
        constexpr const char* APPLICATION_JSON              = "application/json";
        constexpr const char* APPLICATION_XML               = "application/xml";
        constexpr const char* APPLICATION_FORM_URLENCODED   = "application/x-www-form-urlencoded";
        constexpr const char* MULTIPART_FORM_DATA           = "multipart/form-data";
        constexpr const char* TEXT                          = "text/";
        constexpr const char* TEXT_PLAIN                    = "text/plain";
        constexpr const char* TEXT_HTML                     = "text/html";
        constexpr const char* TEXT_CSS                      = "text/css";
        constexpr const char* TEXT_CSV                      = "text/csv";
        constexpr const char* APPLICATION_JAVASCRIPT        = "application/javascript";
        constexpr const char* APPLICATION_PDF               = "application/pdf";
        constexpr const char* IMAGE_PNG                     = "image/png";
        constexpr const char* IMAGE_JPEG                    = "image/jpeg";
        constexpr const char* IMAGE_GIF                     = "image/gif";
        constexpr const char* IMAGE_SVG_XML                 = "image/svg+xml";
        constexpr const char* APPLICATION_OCTET_STREAM      = "application/octet-stream";
    }
  }
}
