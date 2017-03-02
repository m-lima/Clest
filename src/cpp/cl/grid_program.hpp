#pragma once

#include <fmt/format.h>

#include "clest_program.hpp"

namespace clest {

  class GridProgram : public ClestProgram {
  public:
    GridProgram(float xOffset,
                float yOffset,
                float zOffset,
                float xStep,
                float yStep,
                float zStep,
                unsigned short xSize,
                unsigned short ySize,
                unsigned short zSize) :
      ClestProgram(fmt::format(" -D CONST_OFFSET=(float3){{{:f},{:f},{:f}}}"
                               " -D CONST_STEP=(float3){{{:f},{:f},{:f}}}"
                               " -D CONST_SIZE_X={:d}"
                               " -D CONST_SIZE_Y={:d}"
                               " -D CONST_SIZE_Z={:d}",
                               " -D CONST_POINT_COUNT={:ud}",
                               xOffset,
                               yOffset,
                               zOffset,
                               xStep,
                               yStep,
                               zStep,
                               xSize,
                               ySize,
                               zSize)) {}

    inline const char * const path() const override {
      return "opencl/grider.cl";
    }

    inline const char * const name() const override {
      return "grider";
    }

    inline static const char * const NAME() {
      return "grider";
    }

  };
}
