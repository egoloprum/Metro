#pragma once

#include <iostream>
#include <chrono>
#include <functional>
#include <string>
#include <sstream>
#include <iomanip>

#include "context.h"
#include "types.h"

namespace Metro {
  namespace Middlewares {
      using namespace Types;

      inline Middleware logger() {
          constexpr const char * GREEN  = "\033[32m";
          constexpr const char * RED    = "\033[31m";
          constexpr const char * GRAY   = "\033[90m";
          constexpr const char * YELLOW = "\033[33m";
          constexpr const char * BLUE   = "\033[34m";
          constexpr const char * RESET  = "\033[0m";


          return [](Context& context, Next next) {
              using clock = std::chrono::high_resolution_clock;
  
              std::cout
                  << "--> "
                  << context.req._method << " "
                  << context.req._path
                  << std::endl;
  
              auto start = clock::now();
              next();
              auto end = clock::now();

              auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
                  end - start
              ).count();

              std::string time_str;
              std::stringstream ss;
              ss << std::fixed << std::setprecision(3);

              double duration_ms = static_cast<double>(duration_us) / 1'000.0;
              
              if (duration_ms < 1'000.0) {
                  ss << duration_ms << " ms";
              } else {
                  double duration_s = duration_ms / 1'000.0;
                  ss << duration_s << " s";
              }
              
              time_str = ss.str();
  
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
                  << time_str
                  << std::endl;
          };
      }
  }
}
