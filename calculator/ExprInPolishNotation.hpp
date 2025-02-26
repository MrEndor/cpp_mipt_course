#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "AbstractToken.hpp"
#include "BracketToken.hpp"
#include "InvalidExpression.hpp"
#include "OperatorToken.hpp"

namespace internal {
class RawToken : public AbstractToken {
 public:
  RawToken(char raw) : AbstractToken(std::string(1, raw)) {}

  RawToken(const std::string& raw) : AbstractToken(raw) {}

  void ChangePriorityToUnary();

  bool IsMayUnary() const;

  bool IsUnary() const { return GetPriority() == tokens::Priorities::Unary; };

  bool IsBinary() const {
    return !IsUnary() && (GetPriority() > tokens::Priorities::Value);
  }

  bool IsOpenBracket() const {
    return GetPriority() == tokens::Priorities::OpenBracket;
  }
};

void RawToken::ChangePriorityToUnary() {
  if (!IsMayUnary()) {
    return;
  }
  ChangePriority(tokens::Priorities::Unary);
}

bool RawToken::IsMayUnary() const {
  return GetPriority() == tokens::Priorities::Sum ||
         GetPriority() == tokens::Priorities::Subtract;
}
};  // namespace internal

template <typename T>
class ExprInPolishNotation {
 public:
  ExprInPolishNotation(const std::string& expression);

  const std::vector<AbstractToken*>& GetTokens() { return tokens_; }

 private:
  void ProcessOperator(internal::RawToken& current,
                       const internal::RawToken& next);
  void ProcessBracket(const internal::RawToken& current);
  void PostProcess();
  void AppendNumber(std::string& raw_number,
                    std::vector<internal::RawToken>& tokens);
  std::vector<internal::RawToken> Parse(const std::vector<char>& raw_tokens);

  std::vector<AbstractToken*> tokens_;
  std::stack<AbstractToken*> prev_operations_;
};

template <typename T>
void ExprInPolishNotation<T>::PostProcess() {
  while (!prev_operations_.empty()) {
    tokens_.push_back(prev_operations_.top());
    prev_operations_.pop();
  }
  std::reverse(tokens_.begin(), tokens_.end());
}

template <typename T>
void ExprInPolishNotation<T>::ProcessBracket(
    const internal::RawToken& current) {
  if (current.GetPriority() == tokens::Priorities::CloseBracket) {
    prev_operations_.push(new BracketToken(current.GetStringToken()));
    return;
  }
  while (!prev_operations_.empty() &&
         !IsCloseBracketToken(prev_operations_.top())) {
    tokens_.push_back(prev_operations_.top());
    prev_operations_.pop();
  }
  if (prev_operations_.empty()) {
    while (!tokens_.empty()) {
      delete tokens_.back();
      tokens_.pop_back();
    }
    throw InvalidExpression();
  }
  delete prev_operations_.top();
  prev_operations_.pop();
}

template <typename T>
void ExprInPolishNotation<T>::ProcessOperator(internal::RawToken& current,
                                              const internal::RawToken& next) {
  AbstractToken* operation;
  if (current.IsMayUnary() && (next.IsBinary() || next.IsOpenBracket())) {
    operation = new OperatorToken<T, false>(current.GetStringToken());
    current.ChangePriorityToUnary();
  } else {
    operation = new OperatorToken<T, true>(current.GetStringToken());
  }

  while (!prev_operations_.empty() &&
         ((!current.IsUnary() &&
           prev_operations_.top()->GetPriority() >= current.GetPriority()))) {
    tokens_.push_back(prev_operations_.top());
    prev_operations_.pop();
  }
  prev_operations_.push(operation);
}

template <typename T>
void ExprInPolishNotation<T>::AppendNumber(
    std::string& raw_number, std::vector<internal::RawToken>& tokens) {
  if (raw_number.empty()) {
    return;
  }
  std::reverse(raw_number.begin(), raw_number.end());
  tokens.emplace_back(raw_number);
  raw_number.clear();
}

template <typename T>
std::vector<internal::RawToken> ExprInPolishNotation<T>::Parse(
    const std::vector<char>& raw_tokens) {
  std::string raw_number;
  std::vector<internal::RawToken> tokens;

  for (const char& symbol : raw_tokens) {
    if (std::isspace(symbol) != 0) {
      continue;
    }
    internal::RawToken raw_token{symbol};

    if (raw_token.GetPriority() == tokens::Priorities::Value) {
      raw_number.append(raw_token.GetStringToken());
      continue;
    }
    AppendNumber(raw_number, tokens);
    tokens.push_back(raw_token);
  }
  AppendNumber(raw_number, tokens);

  return tokens;
}

template <typename T>
ExprInPolishNotation<T>::ExprInPolishNotation(const std::string& expression) {
  auto raw_tokens =
      Parse(std::vector<char>(expression.rbegin(), expression.rend()));

  for (auto token_iter = raw_tokens.begin(); token_iter != raw_tokens.end();
       ++token_iter) {
    auto current = *token_iter;

    if (current.GetPriority() == tokens::Priorities::Value) {
      tokens_.push_back(new OperandToken<T>(current.GetStringToken()));
      continue;
    }

    if (current.GetPriority() > tokens::Priorities::Value) {
      auto copy = token_iter;
      ProcessOperator(current, *++copy);
    } else {
      ProcessBracket(current);
    }
  }
  PostProcess();
}