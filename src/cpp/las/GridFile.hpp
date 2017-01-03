#pragma once

#include <string>
#include <vector>

#include "LASFile.hpp"

namespace grid {

  class GridFile {
  public:
    GridFile() = default;
    GridFile(const std::string & path) {
      load(path);
    }
    template<int N>
    GridFile(const las::LASFile<N> & lasFile,
             unsigned short sizeX,
             unsigned short sizeY,
             unsigned short sizeZ) {
      convert(lasFile, sizeX, sizeY, sizeZ);
    }

    void save(std::string path) const;
    void load(const std::string & path);

    template<int N>
    void convert(const las::LASFile<N> & lasFile,
                 //uint32_t sizeX,
                 //uint32_t sizeY,
                 //uint32_t sizeZ);
                 unsigned short sizeX,
                 unsigned short sizeY,
                 unsigned short sizeZ);

    uint32_t sizeX() { return mHeader.sizeX; }
    uint32_t sizeY() { return mHeader.sizeY; }
    uint32_t sizeZ() { return mHeader.sizeZ; }

    uint32_t & voxel(unsigned int x, unsigned int y, unsigned int z) {
      return mData[z + y * mHeader.sizeZ + x * mHeader.sizeY * mHeader.sizeZ];
    }

  private:
#pragma pack(push, 1)
    struct GridHeader {
      //uint8_t version;
      //uint32_t sizeX;
      //uint32_t sizeY;
      //uint32_t sizeZ;
      unsigned short sizeX;
      unsigned short sizeY;
      unsigned short sizeZ;
      double xFactor = 1;
      double yFactor = 1;
      double zFactor = 1;
      unsigned short maxValue;
    };
#pragma pack(pop)

    std::vector<uint32_t> mData;
    GridHeader mHeader;

  };

}