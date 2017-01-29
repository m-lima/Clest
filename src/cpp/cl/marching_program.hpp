#pragma once

#include "clest_program.hpp"
#include <fmt\format.h>

namespace clest {

  class MarchingProgram : public ClestProgram {
  public:
    MarchingProgram(unsigned int iso) :
      ClestProgram(fmt::format(" -D CONST_ISO={:d}",
                               iso)) {}

    inline const char * const path() const override {
      return "opencl/marching.cl";
    }

    inline const char * const name() const override {
      return "marching";
    }

    inline static const char * const NAME() {
      return "marching";
    }

  };
}