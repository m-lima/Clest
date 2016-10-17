#include <string>
#include <fstream>
#include <fmt/ostream.h>
#include "LASFile.hpp"

namespace {
  constexpr uint32_t BUFFER_SIZE = 8192;

  void _cleanupHeader(las::PublicHeader & header) {
    if (header.headerSize < sizeof(las::PublicHeader)) {
      char * rawBytes = reinterpret_cast<char*>(&header);
      for (int i = header.headerSize + 1; i < sizeof(las::PublicHeader); i++) {
        rawBytes[i] = 0;
      }
    }
  }

  template <typename IncomingType, typename T>
  uint64_t _loadData(
    std::ifstream & in,
    std::deque<T> & container,
    uint64_t max,
    uint32_t minX,
    uint32_t maxX,
    uint32_t minY,
    uint32_t maxY,
    uint32_t minZ,
    uint32_t maxZ
  ) {
    container.clear();
    uint64_t count = 0;
    uint64_t iCount = 0;
    while (in.good()) {
      IncomingType data[BUFFER_SIZE];
      in.read(reinterpret_cast<char*>(&data), sizeof(IncomingType) * BUFFER_SIZE);
      for (auto & datum : data) {
        if (count < max) {
          count++;
          if (datum.base.x < minX || datum.base.y < minY || datum.base.z < minZ
              || datum.base.x >= maxX || datum.base.y >= maxY || datum.base.z >= maxZ) {
            continue;
          }

          container.push_front(T(datum.base));
          iCount++;
        } else {
          return iCount;
        }
      }
    }
    return iCount;
  }

  template <typename IncomingType, typename T>
  uint64_t _loadData(
    std::ifstream & in,
    std::deque<T> & container,
    uint64_t max
  ) {
    container.clear();
    uint64_t i = 0;
    while (in.good()) {
      IncomingType data[BUFFER_SIZE];
      in.read(reinterpret_cast<char*>(&data), sizeof(IncomingType) * BUFFER_SIZE);
      for (auto & datum : data) {
        if (i < max) {
          container.push_front(T(datum.base));
          i++;
        } else {
          return i;
        }
      }
    }
    return i;
  }
}

namespace las {
  LASFile::LASFile(const std::string & file)
    : _filePath(std::move(file)) {}

  void LASFile::loadHeaders() {
    std::ifstream fileStream(_filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", _filePath));
    }

    fileStream.read(reinterpret_cast<char*>(&publicHeader), sizeof(PublicHeader));
    _cleanupHeader(publicHeader);
    fileStream.seekg(publicHeader.headerSize);

    if (publicHeader.numberOfVariableLengthRecords > 0) {
      recordHeaders.resize(publicHeader.numberOfVariableLengthRecords);

      char buffer[0xFFFF];
      for (auto & header : recordHeaders) {
        fileStream.read(reinterpret_cast<char*>(&header), sizeof(RecordHeader::RAW_SIZE));
        fileStream.read(buffer, header.recordLengthAfterHeader);
        header.data.reserve(header.recordLengthAfterHeader);
        std::memcpy(buffer, header.data.data(), header.recordLengthAfterHeader);
      }
    }
  }

  uint64_t LASFile::loadAllData() {
    std::ifstream fileStream(_filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", _filePath));
    }

    fileStream.seekg(publicHeader.offsetToPointData);
    uint64_t pointDataCount = publicHeader.legacyNumberOfPointRecords > 0
      ? publicHeader.legacyNumberOfPointRecords
      : publicHeader.numberOfPointRecords;

    uint64_t size = _loadData<PointDataTwo>(fileStream, pointData, pointDataCount);

    fileStream.close();
    return size;
  }

  uint64_t LASFile::loadChunk(
    uint32_t minX,
    uint32_t maxX,
    uint32_t minY,
    uint32_t maxY,
    uint32_t minZ,
    uint32_t maxZ
  ) {
    std::ifstream fileStream(_filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", _filePath));
    }

    fileStream.seekg(publicHeader.offsetToPointData);
    uint64_t pointDataCount = publicHeader.legacyNumberOfPointRecords > 0
      ? publicHeader.legacyNumberOfPointRecords
      : publicHeader.numberOfPointRecords;

    uint64_t size = _loadData<PointDataTwo>(
      fileStream,
      pointData,
      pointDataCount,
      minX, 
      maxX,
      minY,
      maxY,
      minZ,
      maxZ
      );

    fileStream.close();
    return size;
  }

  bool isLasValid(const PublicHeader & header) {
    char reference[] = "LASF";
    for (int i = 0; i < header.fileSignature.size(); i++) {
      if (header.fileSignature[i] != reference[i]) {
        return false;
      }
    }

    if (header.headerSize < 227) {
      return false;
    }

    return true;
  }

}
