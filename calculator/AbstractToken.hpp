#pragma once

#include <string>
#include <unordered_map>

#include "AbstractToken.hpp"

namespace tokens {
enum class Priorities {
  CloseBracket = -2,
  OpenBracket = -1,
  Value = 0,
  Sum = 1,
  Subtract = 2,
  Multiplication = 3,
  Divide = 3,
  Unary = 4,
};
}
namespace internal {
const std::unordered_map<std::string, tokens::Priorities> kPriorities = {
    {"+", tokens::Priorities::Sum},
    {"-", tokens::Priorities::Subtract},
    {"*", tokens::Priorities::Multiplication},
    {"/", tokens::Priorities::Divide},
    {"(", tokens::Priorities::OpenBracket},
    {")", tokens::Priorities::CloseBracket},
};

inline tokens::Priorities MatchPriority(const std::string& token) {
  if (!kPriorities.contains(token)) {
    return tokens::Priorities::Value;
  }
  return kPriorities.at(token);
}
}  // namespace internal

class AbstractToken {
 public:
  virtual ~AbstractToken() = default;

  AbstractToken(const std::string& view)
      : view_(view), priority_(internal::MatchPriority(view_)) {}

  const std::string& GetStringToken() const { return view_; }

  const tokens::Priorities& GetPriority() const { return priority_; }

  void UpdateStringToken(const std::string& view);

 protected:
  void ChangePriority(tokens::Priorities priority) { priority_ = priority; }

 private:
  std::string view_;
  tokens::Priorities priority_;
};

void AbstractToken::UpdateStringToken(const std::string& view) {
  view_ = view;
  priority_ = internal::MatchPriority(view_);
}