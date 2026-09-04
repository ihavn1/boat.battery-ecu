#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

void setup();

int main() {
  setup();
  return 0;
}

class String : public std::string {
 public:
  using std::string::operator=;
  using std::string::string;

  String() = default;
  String(const std::string& value) : std::string(value) {}

  String operator+(const char* suffix) const {
    return String(static_cast<const std::string&>(*this) + suffix);
  }

  String operator+(const String& suffix) const {
    return String(static_cast<const std::string&>(*this) + suffix);
  }
};

inline String operator+(const char* prefix, const String& suffix) {
  return String(prefix + static_cast<const std::string&>(suffix));
}

template <typename T>
constexpr T constrain(T value, T minimum, T maximum) {
  return value < minimum ? minimum : (value > maximum ? maximum : value);
}

inline unsigned long millis() {
  static const auto start = std::chrono::steady_clock::now();
  const auto elapsed = std::chrono::steady_clock::now() - start;
  return static_cast<unsigned long>(
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

inline void delay(unsigned long) {}