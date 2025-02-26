#pragma once

#include <exception>

struct InvalidExpression : std::exception {
  [[nodiscard]] const char* what() const noexcept override {
    return "Invalid expression!";
  }
};