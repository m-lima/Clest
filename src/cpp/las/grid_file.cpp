#include <fstream>

#include <clest/util.hpp>
#include <clest/ostream.hpp>

#include "grid_file.hpp"

namespace grid {

  struct LargeColor {
    uint64_t red;
    uint64_t green;
    uint64_t blue;

    Color normalize(uint32_t max) {
      return Color {
        static_cast<uint8_t>(red / max),
        static_cast<uint8_t>(green / max),
        static_cast<uint8_t>(blue / max)
      };
    }
  };

  template<bool C>
  struct ColorUpdater {
    template<int N>
    static void update(const las::PointData<N> * const, LargeColor &) {}
  };

  template<>
  struct ColorUpdater<true> {
    template<int N>
    static void update(const las::PointData<N> * const point,
                       LargeColor & color) {
      color.red += point->red;
      color.green += point->green;
      color.blue += point->blue;
    }
  };

  void GridFile::save(std::string path) const {
    clest::guaranteeNewFile(path, "grid");

    // Prepare for writing
    std::ofstream fileStream(path, std::ofstream::binary);

    if (!fileStream.is_open()) {
      throw clest::Exception::build("Could not open file {}", path);
    }

    // Write the header. It is safe to const_cast because this will
    // not alter the data
    fileStream.write(
      reinterpret_cast<char*>(const_cast<GridHeader*>(&mHeader)),
      sizeof(GridHeader));

    // No buffering being done. The vector is written directly
    fileStream.write(
      reinterpret_cast<const char*>(&mData[0]),
      mData.size() * sizeof(uint32_t));

    // Only write color if the data exists
    if (mHeader.colorized) {
      fileStream.write(
        reinterpret_cast<const char*>(&mColor[0]),
        mColor.size() * sizeof(Color));
    }

    fileStream.close();

  }

  /// Load the grid from file
  /// Integrity will be checked with regards to the values of the header
  void GridFile::load(const std::string & path) {

    // Cannot proceed if the file is not readable
    std::ifstream fileStream(path, std::ifstream::binary);

    if (!fileStream.is_open()) {
      throw clest::Exception::build("Could not open file {}\n", path);
    }

    // Read directly into the header variable
    fileStream.read(reinterpret_cast<char*>(&mHeader), sizeof(GridHeader));

    // Check magic number
    {
      char reference[] = "GRID";
      for (int i = 0; i < mHeader.fileSignature.size(); i++) {
        if (mHeader.fileSignature[i] != reference[i]) {
          throw clest::Exception::build("The file {} seems to be corrupted",
                                        path);
        }
      }
    }

    // Check the integrity of the header
    if (mHeader.sizeX == 0
        || mHeader.sizeY == 0
        || mHeader.sizeZ == 0) {
      throw clest::Exception::build("The file {} seems to be corrupted", path);
    }

    // Prepare the memory to receive the grid from the file
    mData.resize(mHeader.sizeX * mHeader.sizeY * mHeader.sizeZ);

    // Load directly
    fileStream.read(reinterpret_cast<char*>(&mData[0]),
                    mData.size() * sizeof(uint32_t));

    // Only load color if it exists
    if (mHeader.colorized) {
      mColor.resize(mHeader.sizeX * mHeader.sizeY * mHeader.sizeZ);
      fileStream.read(reinterpret_cast<char*>(&mColor[0]),
                      mColor.size() * sizeof(Color));
    }

    fileStream.close();
  }

  /// Converts the give LAS file into a grid
  /// The size of the grid will be `sizeX` * `sizeY` * `sizeZ`
  template<int N>
  void GridFile::convert(las::LASFile<N> & lasFile,
                         uint16_t sizeX,
                         uint16_t sizeY,
                         uint16_t sizeZ
  ) {

    // Check validity of the parameters
    if (sizeX == 0 || sizeY == 0 || sizeZ == 0) {
      throw clest::Exception::build(
        "The size [{}, {}, {}] is invalid and must be larger than zero",
        sizeX,
        sizeY,
        sizeZ);
    }

    // Ensure the header is loaded
    if (!lasFile.isValid()) {
      lasFile.loadHeaders();
    }

    // Update the header
    mHeader.fileSignature[0] = 'G';
    mHeader.fileSignature[1] = 'R';
    mHeader.fileSignature[2] = 'I';
    mHeader.fileSignature[3] = 'D';
    mHeader.colorized = las::PointData<N>::COLORED;
    mHeader.sizeX = sizeX;
    mHeader.sizeY = sizeY;
    mHeader.sizeZ = sizeZ;

    auto deltaAxis =
      std::max(lasFile.publicHeader.maxX - lasFile.publicHeader.minX,
               std::max(lasFile.publicHeader.maxY - lasFile.publicHeader.minY,
                        lasFile.publicHeader.maxZ - lasFile.publicHeader.minZ));

    // Prepare the step sizes for creating the voxels
    double xStep = (lasFile.publicHeader.maxX - lasFile.publicHeader.minX)
      / (sizeX * lasFile.publicHeader.xScaleFactor);
    double xOffset = (lasFile.publicHeader.minX - lasFile.publicHeader.xOffset)
      / (lasFile.publicHeader.xScaleFactor);

    double yStep = (lasFile.publicHeader.maxY - lasFile.publicHeader.minY)
      / (sizeY * lasFile.publicHeader.yScaleFactor);
    double yOffset = (lasFile.publicHeader.minY - lasFile.publicHeader.yOffset)
      / (lasFile.publicHeader.yScaleFactor);

    double zStep = (lasFile.publicHeader.maxZ - lasFile.publicHeader.minZ)
      / (sizeZ * lasFile.publicHeader.zScaleFactor);
    double zOffset = (lasFile.publicHeader.minZ - lasFile.publicHeader.zOffset)
      / (lasFile.publicHeader.zScaleFactor);

    mHeader.xFactor = (lasFile.publicHeader.maxX - lasFile.publicHeader.minX)
      * lasFile.publicHeader.xScaleFactor / deltaAxis;
    mHeader.yFactor = (lasFile.publicHeader.maxY - lasFile.publicHeader.minY)
      * lasFile.publicHeader.yScaleFactor / deltaAxis;
    mHeader.zFactor = (lasFile.publicHeader.maxZ - lasFile.publicHeader.minZ)
      * lasFile.publicHeader.zScaleFactor / deltaAxis;

    // Clear the data vector and preallocate the proper size
    mData = std::vector<uint32_t>(sizeX * sizeY * sizeZ);
    mColor = std::vector<Color>(0);
    if (mHeader.colorized) {
      mColor.reserve(mData.size());
    }
    std::vector<LargeColor> colors(mHeader.colorized ? mData.size() : 0);

    // Iterate and increment the voxel values accordingly
    uint16_t localX;
    uint16_t localY;
    uint16_t localZ;
    uint32_t max = 0;

    // Loading
    {
      std::ifstream in(lasFile.filePath, std::ifstream::binary);
      if (!in.is_open()) {
        throw clest::Exception::build("Could not open file {}",
                                      lasFile.filePath);
      }

      constexpr uint16_t BUFFER_SIZE = 8192;
      uint16_t typeSize = lasFile.publicHeader.pointDataRecordLength;
      uint16_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % typeSize);
      las::PointData<N> *base;
      char data[BUFFER_SIZE];
      uint64_t count = 0;
      uint64_t expected = lasFile.pointDataCount();

      while (count < expected && in.good()) {
        in.read(data, blockSize);

        for (size_t i = 0; i < blockSize && count < expected; i += typeSize) {
          base = reinterpret_cast<las::PointData<N>*>(data + i);
          count++;

          localX = static_cast<uint16_t>((base->x - xOffset) / xStep);
          localY = static_cast<uint16_t>((base->y - yOffset) / yStep);
          localZ = static_cast<uint16_t>((base->z - zOffset) / zStep);

          if (localX == sizeX) localX--;
          if (localY == sizeY) localY--;
          if (localZ == sizeZ) localZ--;

          auto localIndex = index(localX, localY, localZ);

          ColorUpdater<las::PointData<N>::COLORED>::update(base,
                                                           colors[localIndex]);

          if (mData[localIndex] < 0xFFFF) {
            if ((mData[localIndex]++) > max) {
              max++;
            }
          }
        }
      }
    }
    
    if (mHeader.colorized) {
      for (size_t i = 0; i < mData.size(); ++i) {
        mColor.emplace_back(colors[i].normalize(mData[i]));
      }
    }

    mHeader.maxValue = max;
  }

#define __DECLARE_TEMPLATES(index)\
  template void GridFile::convert(las::LASFile<index> & lasFile,\
                                  uint16_t sizeX,\
                                  uint16_t sizeY,\
                                  uint16_t sizeZ);

  __DECLARE_TEMPLATES(-3)
  __DECLARE_TEMPLATES(-2)
  __DECLARE_TEMPLATES(-1)
  __DECLARE_TEMPLATES(0)
  __DECLARE_TEMPLATES(1)
  __DECLARE_TEMPLATES(2)
  __DECLARE_TEMPLATES(3)

#undef __DECLARE_TEMPLATES

}
