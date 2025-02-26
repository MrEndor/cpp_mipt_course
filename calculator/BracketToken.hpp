#pragma once

#include <string>

#include "AbstractToken.hpp"

class BracketToken : public AbstractToken {
 public:
  BracketToken(const std::string& view) : AbstractToken(view) {}

  bool IsOpening() const { return GetStringToken() == "("; };
};

bool IsOpenBracketToken(AbstractToken* token) {
  auto* bracket_token = dynamic_cast<BracketToken*>(token);
  return bracket_token != nullptr && bracket_token->IsOpening();
}

bool IsCloseBracketToken(AbstractToken* token) {
  auto* bracket_token = dynamic_cast<BracketToken*>(token);
  return bracket_token != nullptr && !bracket_token->IsOpening();
}