#pragma once
#include <functional>
#include "context.h"

using Handler = std::function<void(Context&)>;
