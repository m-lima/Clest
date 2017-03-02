#pragma once

#include <string>
#include <vector>

#include "las_file.hpp"

namespace grid {

  struct Color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };

  class GridFile {
  public:
    GridFile() = default;
    GridFile(const std::string & path) {
      load(path);
    }
    template<int N>
    GridFile(las::LASFile<N> && lasFile,
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
    const uint32_t maxValue() const { return mHeader.maxValue; }
    const bool colorized() const { return mHeader.colorized; }

    inline size_t index(unsigned int x, unsigned int y, unsigned int z) const {
      return z + y * mHeader.sizeZ + x * mHeader.sizeY * mHeader.sizeZ;
      //return x + y * mHeader.sizeX + z * mHeader.sizeX * mHeader.sizeY;
    }

    uint32_t & data(unsigned int x, unsigned int y, unsigned int z) {
      return mData[index(x, y, z)];
    }

    uint32_t cData(unsigned int x, unsigned int y, unsigned int z) const {
      return mData[index(x, y, z)];
    }

    const std::vector<uint32_t> * cDataRaw() const {
      return &mData;
    }

    std::vector<uint32_t> * dataRaw() {
      return &mData;
    }

    Color & color(unsigned int x, unsigned int y, unsigned int z) {
      return mColor[index(x, y, z)];
    }

    Color cColor(unsigned int x, unsigned int y, unsigned int z) const {
      return mColor[index(x, y, z)];
    }

    const std::vector<Color> * cColorRaw() const {
      return &mColor;
    }

    std::vector<Color> * colorRaw() {
      return &mColor;
    }

  private:
#pragma pack(push, 1)
    struct GridHeader {
      std::array<char, 4> fileSignature;
      bool colorized : 1;
      uint16_t sizeX;
      uint16_t sizeY;
      uint16_t sizeZ;
      double xFactor = 1;
      double yFactor = 1;
      double zFactor = 1;
      uint32_t maxValue;
    };
#pragma pack(pop)

    std::vector<uint32_t> mData = std::vector<uint32_t>(0);
    std::vector<Color> mColor = std::vector<Color>(0);
    GridHeader mHeader;

  };

}
