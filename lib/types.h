#pragma once

#include <functional>

#include "context.h"

using Next = std::function<void()>;
using Handler = std::function<void(Context&)>;
using Middleware = std::function<void(Context&, Next)>;
