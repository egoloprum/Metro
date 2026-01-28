#pragma once

#include <functional>

#include "context.h"
#include "../lib/json.hpp"

namespace Metro {
  struct Context;
}

namespace Metro {
  namespace Types {
    using Next = std::function<void()>;
    using Handler = std::function<void(Context&)>;
    using Middleware = std::function<void(Context&, Next)>;
    using Json = nlohmann::json;
  }
}

