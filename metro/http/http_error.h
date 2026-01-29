#pragma once

#include <stdexcept>
#include <string>

#include "constants.h"

namespace Metro {

class HttpError : public std::runtime_error {
  public:
    explicit HttpError(int status, std::string message)
      : std::runtime_error(std::move(message)), _status(status) {}

    int status() const noexcept { return _status; }

  private:
    int _status;
  };
}
