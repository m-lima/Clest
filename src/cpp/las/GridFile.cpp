#include <fstream>
#include <fmt/ostream.h>

#include "GridFile.hpp"

namespace grid {

  void GridFile::save(std::string path) const {
    // Check if the file exists
    // Keep appending ".new" before the extension until the filename is unique
    {
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
        auto index = path.rfind(".grid");

        // If it has no extension, first try adding an extension to it
        if (index == std::string::npos) { path += ".grid"; }
        // If it does, append ".new" before the extension
        else { path = path.substr(0, index) + ".new.grid"; }

        counter++;
      } while (counter < 20);

      if (counter == 20) {
        auto message = fmt::format("Could not write to file. Too many copies exist");
        fmt::print(stderr, message);
        throw std::runtime_error(message);
      }
    }

    // Prepare for writing
    std::ofstream fileStream(path, std::ofstream::binary);

    if (!fileStream.is_open()) {
      auto message = fmt::format("Could not open file {}", path);
      fmt::print(stderr, message);
      throw std::runtime_error(message);
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

    fileStream.close();

  }

  /// Load the grid from file
  /// Integrity will be checked with regards to the values of the header
  void GridFile::load(const std::string & path) {

    // Cannot proceed if the file is not readable
    std::ifstream fileStream(path, std::ifstream::binary);

    if (!fileStream.is_open()) {
      auto message = fmt::format("Could not open file {}\n", path);
      fmt::print(stderr, message);
      throw std::runtime_error(message);
    }

    // Read directly into the header variable
    fileStream.read(reinterpret_cast<char*>(&mHeader), sizeof(GridHeader));

    // Check the integrity of the header
    if (mHeader.sizeX == 0
    //if (mHeader.version == 0
    //    || mHeader.sizeX == 0
        || mHeader.sizeY == 0
        || mHeader.sizeZ == 0) {
      auto message = fmt::format("The file {} seems to be corrupted\n", path);
      fmt::print(stderr, message);
      throw std::runtime_error(message);
    }

    // Prepare the memory to receive the grid from the file
    mData.resize(mHeader.sizeX * mHeader.sizeY * mHeader.sizeZ);

    // Load directly
    fileStream.read(reinterpret_cast<char*>(&mData[0]),
                    mData.size() * sizeof(uint32_t));

    fileStream.close();
  }

  /// Converts the give LAS file into a grid
  /// The size of the grid will be `sizeX` * `sizeY` * `sizeZ`
  template<int N>
  void GridFile::convert(const las::LASFile<N> & lasFile,
                         unsigned short sizeX,
                         unsigned short sizeY,
                         unsigned short sizeZ
                         //uint32_t sizeX,
                         //uint32_t sizeY,
                         //uint32_t sizeZ
  ) {

    // Check validity of the parameters
    if (sizeX == 0 || sizeY == 0 || sizeZ == 0) {
      auto message = fmt::format(
        "The size [{}, {}, {}] is invalid and must be larger than zero",
        sizeX,
        sizeY,
        sizeZ);
      fmt::print(stderr, message);
      throw std::runtime_error(message);
    }

    // Update the header
    //mHeader.version = 1;
    mHeader.sizeX = sizeX;
    mHeader.sizeY = sizeY;
    mHeader.sizeZ = sizeZ;

    // Prepare the step sizes for creating the voxels
    double xStep = (lasFile.publicHeader.maxX - lasFile.publicHeader.minX)
      / (sizeX * lasFile.publicHeader.xScaleFactor);
    double xOffset = (lasFile.publicHeader.minX - lasFile.publicHeader.xOffset)
      / (lasFile.publicHeader.xOffset);

    double yStep = (lasFile.publicHeader.maxY - lasFile.publicHeader.minY)
      / (sizeY * lasFile.publicHeader.yScaleFactor);
    double yOffset = (lasFile.publicHeader.minY - lasFile.publicHeader.yOffset)
      / (lasFile.publicHeader.yOffset);

    double zStep = (lasFile.publicHeader.maxZ - lasFile.publicHeader.minZ)
      / (sizeZ * lasFile.publicHeader.zScaleFactor);
    double zOffset = (lasFile.publicHeader.minZ - lasFile.publicHeader.zOffset)
      / (lasFile.publicHeader.zOffset);

    // Clear the data vector and preallocate the proper size
    mData = std::vector<uint32_t>(sizeX * sizeY * sizeZ);

    // Iterate and increment the voxel values accordingly
    uint32_t localX;
    uint32_t localY;
    uint32_t localZ;
    uint64_t max = 0;
    for (auto point : lasFile.pointData) {
      localX = static_cast<uint32_t>((point.x - xOffset) / xStep);
      localY = static_cast<uint32_t>((point.y - yOffset) / yStep);
      localZ = static_cast<uint32_t>((point.z - zOffset) / zStep);

      if (localX == sizeX) localX--;
      if (localY == sizeY) localY--;
      if (localZ == sizeZ) localZ--;

      if ((voxel(localX, localY, localZ)++) > max) {
        max++;
      }
    }

    mHeader.maxValue = max > 0xFFFF ? 0xFFFF : static_cast<unsigned short>(max);
  }

#define __DECLARE_TEMPLATES(index)\
  template void GridFile::convert(const las::LASFile<index> & lasFile,\
                                  unsigned short sizeX,\
                                  unsigned short sizeY,\
                                  unsigned short sizeZ);

  __DECLARE_TEMPLATES(-1)
  __DECLARE_TEMPLATES(0)
  __DECLARE_TEMPLATES(1)
  __DECLARE_TEMPLATES(2)
  __DECLARE_TEMPLATES(3)

#undef __DECLARE_TEMPLATES

}
