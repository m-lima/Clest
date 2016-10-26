#ifndef CLEST_COLORIZER_HPP
#define CLEST_COLORIZER_HPP

#include <fmt/ostream.h>
#include <deque>
#include "las/LASFile.hpp"

using namespace las;

namespace {

  template <typename T>
  std::vector<PointData<2>> iterateAndSetColor(const LASFile<T> & file) {
    constexpr uint16_t BUFFER_SIZE = 8192;
    constexpr uint16_t MAX_COLOR = 0xFFFF;

    uint64_t dataPointCount = file.pointDataCount();
    uint16_t typeSize = file.publicHeader.pointDataRecordLength;
    
    uint64_t currentPoint = 0;
    uint16_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % typeSize);
    
    PointData<2> newPoint;
    newPoint.green = 0;
    std::vector<PointData<2>> output;
    output.reserve(dataPointCount);
    T *base;
    char data[BUFFER_SIZE];
    
    std::ifstream fileStream(file.filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", file.filePath));
    }

    fileStream.seekg(file.publicHeader.offsetToPointData);

    while (currentPoint < dataPointCount && fileStream.good()) {
      fileStream.read(data, blockSize);
      for (size_t i = 0; i < blockSize && currentPoint < dataPointCount; i += typeSize) {
        base = reinterpret_cast<T*>(data + i);

        newPoint.x = base->x;
        newPoint.y = base->y;
        newPoint.z = base->z;
        newPoint.red = static_cast<uint16_t>(currentPoint * (MAX_COLOR + 1) / dataPointCount);
        newPoint.blue = static_cast<uint16_t>(MAX_COLOR - (currentPoint * (MAX_COLOR + 1) / dataPointCount));
        output.push_back(newPoint);
        currentPoint++;
      }
    }

    fileStream.close();
    return output;
  }
}

namespace clest {
  template <typename T>
  void colorize(const LASFile<T> & lasFile) {
    if (!isLasValid(lasFile.publicHeader)) {
      fmt::print(stderr, "Trying to colorize LAS file, but {} seems to be corrupted.\n", lasFile.filePath);
    }

    if (lasFile.pointDataCount() < 1) {
      fmt::print(stderr, "Trying to colorize LAS file, but {} seems to be empty.\n", lasFile.filePath);
    }

    LASFile<PointData<2>> newFile(lasFile.filePath + ".color.las");

    newFile.publicHeader = lasFile.publicHeader;
    newFile.publicHeader.pointDataRecordFormat = 2;
    newFile.publicHeader.pointDataRecordLength = sizeof(las::PointData<2>);
    newFile.recordHeaders = lasFile.recordHeaders;

    iterateAndSetColor(lasFile);

    newFile.save();
  }
}
#endif // CLEST_COLORIZER_HPP