#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <cctype>

#include "context.h"

namespace Metro {
  namespace Types {
    using Next = std::function<void()>;
    using Handler = std::function<void(Context&)>;
    using Middleware = std::function<void(Context&, Next)>;
  }
}

