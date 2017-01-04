#pragma once

#include <string>
#include <vector>

#include "las_file.hpp"

namespace grid {

  class GridFile {
  public:
    GridFile() = default;
    GridFile(const std::string & path) {
      load(path);
    }
    template<int N>
    GridFile(const las::LASFile<N> & lasFile,
             uint16_t sizeX,
             uint16_t sizeY,
             uint16_t sizeZ) {
      convert(lasFile, sizeX, sizeY, sizeZ);
    }

    void save(std::string path) const;
    void load(const std::string & path);

    template<int N>
    void convert(const las::LASFile<N> & lasFile,
                 uint16_t sizeX,
                 uint16_t sizeY,
                 uint16_t sizeZ);

    uint16_t sizeX() { return mHeader.sizeX; }
    uint16_t sizeY() { return mHeader.sizeY; }
    uint16_t sizeZ() { return mHeader.sizeZ; }
    uint16_t maxValue() { return mHeader.maxValue; }

    uint16_t & voxel(unsigned int x, unsigned int y, unsigned int z) {
      return mData[z + y * mHeader.sizeZ + x * mHeader.sizeY * mHeader.sizeZ];
    }

  private:
#pragma pack(push, 1)
    struct GridHeader {
      uint16_t sizeX;
      uint16_t sizeY;
      uint16_t sizeZ;
      double xFactor = 1;
      double yFactor = 1;
      double zFactor = 1;
      uint16_t maxValue;
    };
#pragma pack(pop)

    std::vector<uint16_t> mData;
    GridHeader mHeader;

  };

}