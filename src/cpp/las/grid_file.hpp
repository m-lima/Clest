#pragma once

#include <string>
#include <vector>

#include "las_file.hpp"

namespace grid {

#pragma pack(push, 1)
  struct Color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };
#pragma pack(pop)

  class GridFile {
  public:
    GridFile() = default;
    GridFile(const std::string & path) {
      load(path);
    }
    template<int N>
    GridFile(las::LASFile<N> & lasFile,
             uint16_t sizeX,
             uint16_t sizeY,
             uint16_t sizeZ) {
      convert(lasFile, sizeX, sizeY, sizeZ);
    }

    void save(std::string path) const;
    void load(const std::string & path);

    template<int N>
    void convert(las::LASFile<N> & lasFile,
                 uint16_t sizeX,
                 uint16_t sizeY,
                 uint16_t sizeZ);

    const uint16_t sizeX() const { return mHeader.sizeX; }
    const uint16_t sizeY() const { return mHeader.sizeY; }
    const uint16_t sizeZ() const { return mHeader.sizeZ; }
    const uint16_t maxValue() const { return mHeader.maxValue; }

    uint32_t & data(unsigned int x, unsigned int y, unsigned int z) {
      return mData[z + y * mHeader.sizeZ + x * mHeader.sizeY * mHeader.sizeZ];
    }
    
    uint32_t cData(unsigned int x, unsigned int y, unsigned int z) const {
      return mData[z + y * mHeader.sizeZ + x * mHeader.sizeY * mHeader.sizeZ];
    }

    const std::vector<uint32_t> * craw() const {
      return &mData;
    }

    std::vector<uint32_t> * raw() {
      return &mData;
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

    std::vector<uint32_t> mData = std::vector<uint32_t>(0);
    std::vector<Color> mColors = std::vector<Color>(0);
    GridHeader mHeader;

  };

}