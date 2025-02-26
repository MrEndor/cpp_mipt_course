#include "string.hpp"

namespace entrails {
size_t Max(size_t lhs, size_t rhs) { return lhs > rhs ? lhs : rhs; }

size_t Min(size_t lhs, size_t rhs) { return lhs > rhs ? rhs : lhs; }

bool Isspace(char symbol) {
  switch (symbol) {
    case '\n':
    case '\t':
    case '\r':
    case '\v':
    case '\f':
    case ' ':
      return true;
    default:
      return false;
  }
}
}  // namespace entrails

String::String(size_t size, char character)
    : size_(size), capacity_(size + 1), characters_(new char[capacity_]) {
  for (size_t i = 0; i < size_; ++i) {
    characters_[i] = character;
  }
  SetNullSymbol(size_);
}

String::String(const char* raw_string)
    : size_(std::strlen(raw_string)),
      capacity_(size_ + 1),
      characters_(new char[capacity_]) {
  for (size_t i = 0; i < size_; ++i) {
    characters_[i] = raw_string[i];
  }
  SetNullSymbol(size_);
}

String::~String() { delete[] characters_; }

String::String(const String& other)
    : size_(other.size_),
      capacity_(other.capacity_),
      characters_(new char[other.capacity_]) {
  for (size_t i = 0; i < other.size_; ++i) {
    characters_[i] = other[i];
  }
  SetNullSymbol(size_);
}

String& String::operator=(const String& other) {
  if (this == &other) {
    return *this;
  }
  delete[] characters_;
  size_ = other.Size();
  capacity_ = other.Capacity();
  characters_ = new char[capacity_];

  for (size_t i = 0; i < other.Size(); ++i) {
    characters_[i] = other[i];
  }
  if (capacity_ != 0) {
    SetNullSymbol(size_);
  }

  return *this;
}

void String::Clear() {
  size_ = 0;
  if (capacity_ == 0) {
    return;
  }
  SetNullSymbol(size_);
}

void String::PushBack(char character) {
  if (size_ + 1 >= capacity_) {
    Reserve(capacity_ * 2 + 2);
  }

  characters_[size_++] = character;
  SetNullSymbol(size_);
}

void String::PopBack() {
  if (Empty()) {
    return;
  }
  SetNullSymbol(--size_);
}

void String::Resize(size_t new_size) {
  if (new_size > size_) {
    Reserve(entrails::Max(new_size + 1, 2 * capacity_ + 1));
  }

  size_ = new_size;
  SetNullSymbol(size_);
}

void String::Reserve(size_t new_cap) {
  if (new_cap < capacity_) {
    return;
  }

  char* new_data = new char[new_cap];
  for (size_t i = 0; i < size_; ++i) {
    new_data[i] = characters_[i];
  }
  delete[] characters_;
  characters_ = new_data;
  capacity_ = new_cap;
  SetNullSymbol(size_);
}

void String::Resize(size_t new_size, char character) {
  size_t prev_size = size_;
  Resize(new_size);
  for (size_t i = prev_size; i < new_size; ++i) {
    characters_[i] = character;
  }
  SetNullSymbol(size_);
}

void String::ShrinkToFit() {
  capacity_ = size_;
  char* new_data = new char[capacity_];
  for (size_t i = 0; i < size_; ++i) {
    new_data[i] = characters_[i];
  }
  delete[] characters_;
  characters_ = new_data;
}

void String::Swap(String& other) {
  char* temp_data = other.characters_;
  size_t temp_size = other.size_;
  size_t temp_cap = other.capacity_;

  other.characters_ = characters_;
  other.size_ = size_;
  other.capacity_ = capacity_;

  characters_ = temp_data;
  size_ = temp_size;
  capacity_ = temp_cap;
}

char& String::Front() { return characters_[0]; }

char& String::Back() { return characters_[size_ - 1]; }

const char& String::Front() const { return characters_[0]; }

const char& String::Back() const { return characters_[size_ - 1]; }

bool String::Empty() const { return Size() == 0; }

size_t String::Size() const { return size_; }

size_t String::Capacity() const { return capacity_; }

char* String::Data() { return characters_; }

const char* String::Data() const { return characters_; }

char& String::operator[](size_t index) { return characters_[index]; }

const char& String::operator[](size_t index) const {
  return characters_[index];
}

void String::SetNullSymbol(size_t index) { characters_[index] = '\0'; }

std::vector<String> String::Split(const String& delim) {
  if (Empty() || delim.Empty()) {
    return {*this};
  }
  std::vector<String> substrings;

  size_t delim_pointer = 0;
  String buffer;
  for (size_t i = 0; i < size_; ++i) {
    if (characters_[i] == delim[delim_pointer]) {
      ++delim_pointer;
    } else {
      delim_pointer = 0;
    }

    buffer.PushBack(characters_[i]);
    if (delim_pointer != delim.Size()) {
      continue;
    }

    while (delim_pointer != 0) {
      buffer.PopBack();
      --delim_pointer;
    }
    substrings.push_back(buffer);
    buffer.Clear();
  }
  substrings.push_back(buffer);

  return substrings;
}

String String::Join(const std::vector<String>& strings) const {
  if (strings.empty()) {
    return "";
  }
  String new_string = strings[0];

  for (size_t i = 1; i < strings.size(); ++i) {
    new_string += (*this + strings[i]);
  }

  return new_string;
}

String& String::operator+=(const String& other) {
  Reserve(size_ + other.size_);
  for (size_t i = 0; i < other.Size(); ++i) {
    PushBack(other[i]);
  }
  return *this;
}

String& String::operator*=(size_t count) {
  if (count == 0) {
    Clear();
    return *this;
  }
  String copy = *this;
  copy.Reserve(copy.Size() * (count - 1));
  for (size_t i = 0; i < count - 1; ++i) {
    *this += copy;
  }

  return *this;
}

String operator+(const String& lhs, const String& rhs) {
  String new_string = lhs;

  new_string += rhs;

  return new_string;
}

bool operator>(const String& lhs, const String& rhs) {
  size_t size = entrails::Min(lhs.Size(), rhs.Size());
  for (size_t i = 0; i < size; ++i) {
    if (lhs[i] < rhs[i]) {
      return false;
    }
    if (lhs[i] > rhs[i]) {
      return true;
    }
  }
  return lhs.Size() > rhs.Size();
}

bool operator>=(const String& lhs, const String& rhs) { return !(lhs < rhs); }

bool operator<(const String& lhs, const String& rhs) { return rhs > lhs; }

bool operator<=(const String& lhs, const String& rhs) { return rhs >= lhs; }

bool operator==(const String& lhs, const String& rhs) {
  if (lhs.Size() != rhs.Size()) {
    return false;
  }

  for (size_t i = 0; i < lhs.Size(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }

  return true;
}

bool operator!=(const String& lhs, const String& rhs) { return !(lhs == rhs); }

String operator*(const String& string, size_t count) {
  String new_string = string;
  new_string *= count;

  return new_string;
}

std::ostream& operator<<(std::ostream& ostream, const String& string) {
  ostream << string.Data();

  return ostream;
}

std::istream& operator>>(std::istream& istream, String& string) {
  std::istream::sentry sentry(istream);

  if (!sentry) {
    return istream;
  }
  char buf;
  while (istream.get(buf) && !entrails::Isspace(buf)) {
    string.PushBack(buf);
  }

  return istream;
}
