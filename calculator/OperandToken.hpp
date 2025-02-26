#pragma once

#include "AbstractToken.hpp"
#include "sstream"
#include "string"

template <typename T>
class OperandToken : public AbstractToken {
 public:
  OperandToken(const std::string& view);
  OperandToken(const T& value);

  const T& GetValue() const { return value_; }

 private:
  T value_;
};

template <typename T>
OperandToken<T>::OperandToken(const std::string& view) : AbstractToken(view) {
  std::stringstream stream(view);
  stream >> value_;
}

template <typename T>
OperandToken<T>::OperandToken(const T& value)
    : AbstractToken(""), value_(value) {
  std::stringstream stream;
  stream << value;
  UpdateStringToken(stream.str());
}

template <typename T>
bool IsOperand(AbstractToken* token) {
  return dynamic_cast<OperandToken<T>*>(token) != nullptr;
}