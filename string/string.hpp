#include <cstring>
#include <iostream>
#include <vector>

class String {
 public:
  String() = default;
  String(size_t size, char character);
  String(const char* raw_string);

  String(const String&);
  String& operator=(const String& other);
  ~String();

  void PushBack(char character);
  void PopBack();
  void Clear();

  char& operator[](size_t index);
  const char& operator[](size_t index) const;
  char& Front();
  char& Back();
  const char& Front() const;
  const char& Back() const;

  void Resize(size_t new_size);
  void Resize(size_t new_size, char character);
  void Reserve(size_t new_cap);
  void ShrinkToFit();
  void Swap(String& other);

  bool Empty() const;
  size_t Size() const;
  size_t Capacity() const;
  char* Data();
  const char* Data() const;

  std::vector<String> Split(const String& delim = " ");
  String Join(const std::vector<String>& strings) const;

  String& operator+=(const String& other);
  String& operator*=(size_t count);

 private:
  size_t size_ = 0;
  size_t capacity_ = 0;
  char* characters_ = nullptr;

  void SetNullSymbol(size_t index);
};

String operator+(const String& lhs, const String& rhs);
bool operator>(const String& lhs, const String& rhs);
bool operator>=(const String& lhs, const String& rhs);
bool operator<(const String& lhs, const String& rhs);
bool operator<=(const String& lhs, const String& rhs);
bool operator==(const String& lhs, const String& rhs);
bool operator!=(const String& lhs, const String& rhs);
String operator*(const String& string, size_t count);

std::ostream& operator<<(std::ostream& ostream, const String& string);
std::istream& operator>>(std::istream& istream, String& string);
