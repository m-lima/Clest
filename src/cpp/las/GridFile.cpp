#include <fstream>

#include <clest/util.hpp>
#include <clest/ostream.hpp>

#include "GridFile.hpp"

namespace grid {

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
      mData.size() * sizeof(uint16_t));

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

    // Check the integrity of the header
    if (mHeader.sizeX == 0
        || mHeader.sizeY == 0
        || mHeader.sizeZ == 0) {
      throw clest::Exception::build("The file {} seems to be corrupted\n", path);
    }

    // Prepare the memory to receive the grid from the file
    mData.resize(mHeader.sizeX * mHeader.sizeY * mHeader.sizeZ);

    // Load directly
    fileStream.read(reinterpret_cast<char*>(&mData[0]),
                    mData.size() * sizeof(uint16_t));

    fileStream.close();
  }

  /// Converts the give LAS file into a grid
  /// The size of the grid will be `sizeX` * `sizeY` * `sizeZ`
  template<int N>
  void GridFile::convert(const las::LASFile<N> & lasFile,
                         uint16_t sizeX,
                         uint16_t sizeY,
                         uint16_t sizeZ
  ) {

    if (!lasFile.isValid()) {

    }

    // Check validity of the parameters
    if (sizeX == 0 || sizeY == 0 || sizeZ == 0) {
      throw clest::Exception::build(
        "The size [{}, {}, {}] is invalid and must be larger than zero",
        sizeX,
        sizeY,
        sizeZ);
    }

    // Update the header
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
    mData = std::vector<uint16_t>(sizeX * sizeY * sizeZ);

    // Iterate and increment the voxel values accordingly
    uint16_t localX;
    uint16_t localY;
    uint16_t localZ;
    uint32_t max = 0;
    for (auto point : lasFile.pointData) {
      localX = static_cast<uint16_t>((point.x - xOffset) / xStep);
      localY = static_cast<uint16_t>((point.y - yOffset) / yStep);
      localZ = static_cast<uint16_t>((point.z - zOffset) / zStep);

      if (localX == sizeX) localX--;
      if (localY == sizeY) localY--;
      if (localZ == sizeZ) localZ--;

      if ((voxel(localX, localY, localZ)++) > max) {
        max++;
      }
    }

    mHeader.maxValue = max > 0xFFFF ? 0xFFFF : static_cast<uint16_t>(max);
  }

#define __DECLARE_TEMPLATES(index)\
  template void GridFile::convert(const las::LASFile<index> & lasFile,\
                                  uint16_t sizeX,\
                                  uint16_t sizeY,\
                                  uint16_t sizeZ);

  __DECLARE_TEMPLATES(-1)
  __DECLARE_TEMPLATES(0)
  __DECLARE_TEMPLATES(1)
  __DECLARE_TEMPLATES(2)
  __DECLARE_TEMPLATES(3)

#undef __DECLARE_TEMPLATES

}
