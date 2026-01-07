#pragma once

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
