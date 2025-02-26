#pragma once

#include "ExprInPolishNotation.hpp"
#include "OperandToken.hpp"
#include "OperatorToken.hpp"

template <typename T>
class Calculator {
 public:
  static T CalculateExpr(const std::string& expression);

  static void CalculateTokens(std::deque<AbstractToken*>& tokens);
  static void CalculateBinaryOperator(std::deque<AbstractToken*>& tokens);
  static void CalculateUnaryOperator(std::deque<AbstractToken*>& tokens);
};

template <typename T>
T Calculator<T>::CalculateExpr(const std::string& expression) {
  auto tokens = ExprInPolishNotation<T>(expression).GetTokens();
  auto result = std::deque<AbstractToken*>(tokens.begin(), tokens.end());
  CalculateTokens(result);

  if (result.size() == 1) {
    T value = dynamic_cast<OperandToken<T>*>(result.front())->GetValue();
    delete result.front();
    return value;
  }
  while (!result.empty()) {
    delete result.front();
    result.pop_front();
  }
  throw InvalidExpression();
}

template <typename T>
void Calculator<T>::CalculateTokens(std::deque<AbstractToken*>& tokens) {
  if (IsBinaryOperator<T>(tokens.front())) {
    CalculateBinaryOperator(tokens);
  } else if (IsUnaryOperator<T>(tokens.front())) {
    CalculateUnaryOperator(tokens);
  }
}

template <typename T>
void Calculator<T>::CalculateUnaryOperator(std::deque<AbstractToken*>& tokens) {
  AbstractToken* current = tokens.front();
  tokens.pop_front();

  auto* operation = dynamic_cast<OperatorToken<T, false>*>(current);
  CalculateTokens(tokens);

  auto* element = dynamic_cast<OperandToken<T>*>(tokens.front());
  tokens.pop_front();
  tokens.push_front(operation->Calculate(element));

  delete element;
  delete operation;
}

template <typename T>
void Calculator<T>::CalculateBinaryOperator(
    std::deque<AbstractToken*>& tokens) {
  AbstractToken* current = tokens.front();
  tokens.pop_front();

  auto* operation = dynamic_cast<OperatorToken<T, true>*>(current);
  CalculateTokens(tokens);

  auto* lhs = dynamic_cast<OperandToken<T>*>(tokens.front());
  tokens.pop_front();
  CalculateTokens(tokens);

  auto* rhs = dynamic_cast<OperandToken<T>*>(tokens.front());
  tokens.pop_front();
  tokens.push_front(operation->Calculate(lhs, rhs));

  delete rhs;
  delete lhs;
  delete operation;
}