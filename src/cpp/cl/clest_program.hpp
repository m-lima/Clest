#pragma once

#include <string>

namespace clest {

  class ClestProgram {
  public:
    ClestProgram(const std::string & buildString) :
      mBuildString(buildString) {};
    virtual ~ClestProgram() = default;

    const char * const buildString() const {
      return mBuildString.c_str();
    }

    virtual const char * const path() const = 0;
    virtual const char * const name() const = 0;

  private:
    const std::string mBuildString;
  };
}