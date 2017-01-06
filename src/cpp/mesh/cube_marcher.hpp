#pragma once

#include <vector>

namespace mesh {
  class CubeMarcher {
  public:
    CubeMarcher() = default;
    CubeMarcher(unsigned short sizeX,
                unsigned short sizeY,
                unsigned short sizeZ) :
      mSizeX(sizeX),
      mSizeY(sizeY),
      mSizeZ(sizeZ) {}

  private:
    bool usingReference = false;
    unsigned short mSizeX;
    unsigned short mSizeY;
    unsigned short mSizeZ;
    std::vector<uint16_t> mData;
  };
}