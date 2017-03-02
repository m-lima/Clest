#pragma once

#include <algorithm>
#include <functional>
#include <cctype>
#include <fstream>
#include <fmt/format.h>

namespace clest {

  template<bool C>
  class DoIf {
  public:
    template<typename F>
    static inline void execute(F f) {}
  };

  template<>
  class DoIf<true> {
  public:
    template<typename F>
    static inline void execute(F f) { f(); }
  };
  
  inline void trim(std::string & str) {
    // Left trim
    str.erase(
      str.begin(),
      std::find_if(str.begin(),
                   str.end(),
                   std::not1(std::ptr_fun<int, int>(std::isspace)))
    );

    // Right trim
    str.erase(
      std::find_if(str.rbegin(),
                   str.rend(),
                   std::not1(std::ptr_fun<int, int>(std::isspace))).base(), 
      str.end()
    );
  }

  inline const std::string simplifyValue(double value) {
    if (value < 1000) {
      return fmt::format("{:d}", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} thousand", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} million", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} billion", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} trillion", value);
    }

    return "<invalid>";
  }

  //template <bool x64>
  //inline bool isBigEndian() {
  //  if (x64) {
  //    union {
  //      uint64_t i;
  //      char c[8];
  //    } testValue = { 0x0102030405060708 };
  //  } else {
  //    union {
  //      uint32_t i;
  //      char c[4];
  //    } testValue = { 0x01020304 };
  //  }
  //
  //  return testValue.c[0] == 1;
  //}

  inline const char * extractOption(const std::vector<const char*> & args,
                                    const std::string & option) {
    auto itr = std::find(args.cbegin(), args.cend(), option);
    if (itr != args.cend() && ++itr != args.cend()) {
      return *itr;
    }
    return 0;
  }

  inline char * extractOption(char * begin[],
                              char * end[],
                              const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
      return *itr;
    }
    return 0;
  }

  inline bool findOption(const std::vector<const char*> & args,
                         const std::string & option) {
    return std::find(args.cbegin(), args.cend(), option) != args.cend();
  }

  inline bool findOption(char * begin[],
                         char * end[],
                         const std::string & option) {
    return std::find(begin, end, option) != end;
  }

  template <int MaxTries = 20>
  inline void guaranteeNewFile(std::string & path,
                               const std::string & extension) {
    // Check if the file exists
    // Keep appending ".new" before the extension until
    // the filename is unique
    int counter = 0;
    do {

      // Try to read it
      std::ifstream testFile(path);

      // If it's not readable, it doesn't exits; Quit loop
      if (!testFile.is_open()) {
        break;
      }

      testFile.close();

      // Find the extension
      auto index = path.rfind("." + extension);

      // If it has no extension, first try adding an extension to it
      if (index == std::string::npos) {
        path = fmt::format("{}.{}", path, extension);
      }
      // If it does, append ".new" before the extension
      else {
        path = fmt::format("{}.new.{}", path.substr(0, index), extension);
      }

      counter++;
    } while (counter < MaxTries);

    if (counter == MaxTries) {
      auto message = fmt::format(
        "Could not write to file. Too many copies exist");
      fmt::print(stderr, message);
      throw std::runtime_error(message);
    }
  }
}
