#pragma once

#include <fmt/ostream.h>
#include <stdexcept>

namespace clest {
  class Exception : public std::runtime_error {
  public:
    explicit Exception(const std::string & message) :
      std::runtime_error(message) {
      fmt::print(stderr, message + "\n");
    }

    template <typename ... Args>
    static Exception build(const char * format, const Args & ... args) {
      return Exception(fmt::format(format, args ...));
    }
  };

  template <typename ... Args>
  inline void println(const char * format, const Args & ... args) {
    fmt::print(format, args...);
    fmt::print("\n");
  }

  template <typename ... Args>
  inline void println(std::FILE * file,
                      char * format,
                      const Args & ... args) {
    fmt::print(file, format, args...);
    fmt::print(file, "\n");
  }
}