#pragma once

#include <functional>
#include <unordered_map>

#include "AbstractToken.hpp"
#include "OperandToken.hpp"

template <typename T>
using BinaryFunction = std::function<T(const T&, const T&)>;

template <typename T>
using UnaryFunction = std::function<T(const T&)>;

namespace internal {
template <typename T>
const std::unordered_map<std::string, BinaryFunction<T>> kBinaryOperators{
    {"+", [](const T& lhs, const T& rhs) { return lhs + rhs; }},
    {"-", [](const T& lhs, const T& rhs) { return lhs - rhs; }},
    {"*", [](const T& lhs, const T& rhs) { return lhs * rhs; }},
    {"/", [](const T& lhs, const T& rhs) { return lhs / rhs; }},
};

template <typename T>
const std::unordered_map<std::string, UnaryFunction<T>> kUnaryOperators{
    {"+", [](const T& operand) { return +operand; }},
    {"-", [](const T& operand) { return -operand; }},
};
}  // namespace internal

template <typename T, bool IsBinaryOp>
class OperatorToken : public AbstractToken {
 public:
  OperatorToken(const std::string& view) : AbstractToken(view) {}

  bool IsBinary() const { return IsBinaryOp; };

  OperandToken<T>* Calculate(OperandToken<T>* lhs, OperandToken<T>* rhs);
  OperandToken<T>* Calculate(OperandToken<T>* operand);
};

template <typename T, bool IsBinaryOp>
OperandToken<T>* OperatorToken<T, IsBinaryOp>::Calculate(OperandToken<T>* lhs,
                                                         OperandToken<T>* rhs) {
  static_assert(IsBinaryOp);

  T value = internal::kBinaryOperators<T>.at(GetStringToken())(lhs->GetValue(),
                                                               rhs->GetValue());

  return new OperandToken<T>(value);
}

template <typename T, bool IsBinaryOp>
OperandToken<T>* OperatorToken<T, IsBinaryOp>::Calculate(
    OperandToken<T>* operand) {
  static_assert(!IsBinaryOp);

  T value =
      internal::kUnaryOperators<T>.at(GetStringToken())(operand->GetValue());

  return new OperandToken<T>(value);
}

template <typename T>
bool IsBinaryOperator(AbstractToken* token) {
  return dynamic_cast<OperatorToken<T, true>*>(token) != nullptr;
}

template <typename T>
bool IsUnaryOperator(AbstractToken* token) {
  return dynamic_cast<OperatorToken<T, false>*>(token) != nullptr;
}