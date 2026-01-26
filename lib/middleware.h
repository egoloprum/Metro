#pragma once

#include <iostream>
#include <chrono>
#include <functional>
#include <string>

#include "context.h"
#include "types.h"

constexpr const char * GREEN  = "\033[32m";
constexpr const char * RED    = "\033[31m";
constexpr const char * GRAY   = "\033[90m";
constexpr const char * YELLOW = "\033[33m";
constexpr const char * BLUE   = "\033[34m";
constexpr const char * RESET  = "\033[0m";

namespace MIDDLEWARE {
    inline Middleware logger() {
        return [](Context& context, Next next) {
            using clock = std::chrono::high_resolution_clock;

            auto start = clock::now();

            std::cout
                << "--> "
                << context.req._method << " "
                << context.req._path
                << std::endl;

            next();

            auto end = clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                end - start
            ).count();

            const char* color;
            if (context.res._status >= 500) {
                color = RED;          
            } else if (context.res._status >= 400) {
                color = YELLOW;       
            } else if (context.res._status >= 300) {
                color = BLUE;         
            } else if (context.res._status >= 200) {
                color = GREEN;        
            } else {
                color = GRAY;         
            }

            std::cout
                << "<-- "
                << context.req._method << " "
                << context.req._path << " "
                << color << context.res._status << RESET << " "
                << duration << "ms"
                << std::endl;
        };
    }

}
