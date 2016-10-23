#include <string>
#include <fstream>
#include <fmt/ostream.h>
#include "LASFile.hpp"

namespace {
  constexpr uint16_t BUFFER_SIZE = 8192;

  void _cleanupHeader(las::PublicHeader & header) {
    if (header.headerSize < sizeof(las::PublicHeader)) {
      char * rawBytes = reinterpret_cast<char*>(&header);
      for (int i = header.headerSize + 1; i < sizeof(las::PublicHeader); i++) {
        rawBytes[i] = 0;
      }
    }
  }

  template <typename T>
  uint64_t _loadData(
    uint16_t typeSize,
    std::ifstream & in,
    std::deque<T> & container,
    uint64_t max,
    uint32_t minX = -1,
    uint32_t maxX = 0,
    uint32_t minY = -1,
    uint32_t maxY = 0,
    uint32_t minZ = -1,
    uint32_t maxZ = 0
  ) {
    container.clear();
    uint64_t count = 0;
    uint64_t iCount = 0;
    las::PointDataBase *base;

    struct maxers {
      uint32_t minX = -1;
      uint32_t maxX = 0;
      uint32_t minY = -1;
      uint32_t maxY = 0;
      uint32_t minZ = -1;
      uint32_t maxZ = 0;
    } maxers;

    assert(BUFFER_SIZE > typeSize);
    uint16_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % typeSize);

    char data[BUFFER_SIZE];
    if (
      minX == -1 &&
      maxX == 0 &&
      minY == -1 &&
      maxY == 0 &&
      minZ == -1 &&
      maxZ == 0) {
      while (count < max && in.good()) {
        in.read(data, blockSize);
        for (size_t i = typeSize; i < blockSize; i += typeSize) {
          base = reinterpret_cast<las::PointDataBase*>(data + i - typeSize);
          if (count < max) {
            count++;
            container.push_back(T(*base));
            if (maxers.minX > base->x) maxers.minX = base->x;
            if (maxers.minY > base->y) maxers.minY = base->y;
            if (maxers.minZ > base->z) maxers.minZ = base->z;
            if (maxers.maxX < base->x) maxers.maxX = base->x;
            if (maxers.maxY < base->y) maxers.maxY = base->y;
            if (maxers.maxZ < base->z) maxers.maxZ = base->z;
            iCount++;
          } else {
            break;
          }
        }
      }
    } else {
      while (count < max && in.good()) {
        in.read(data, blockSize);
        for (size_t i = typeSize; i < blockSize; i += typeSize) {
          base = reinterpret_cast<las::PointDataBase*>(data + i - typeSize);
          if (count < max) {
            count++;
            if (base->x < minX || base->y < minY || base->z < minZ
                || base->x >= maxX || base->y >= maxY || base->z >= maxZ) {
              continue;
            }
            container.push_back(T(*base));
            iCount++;
          } else {
            break;
          }
        }
      }
    }
    container.shrink_to_fit();
    fmt::print("X: [{}, {}]\nY: [{}, {}]\nZ: [{}, {}]\n", maxers.minX, maxers.maxX, maxers.minY, maxers.maxY, maxers.minZ, maxers.maxZ);
    return iCount;
  }
}

namespace las {
  
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

  template <typename T>
  LASFile<T>::LASFile(const std::string & file)
    : filePath(std::move(file)) {}

  template <typename T>
  void LASFile<T>::loadHeaders() {
    std::ifstream fileStream(filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", filePath));
    }

    fileStream.read(reinterpret_cast<char*>(&publicHeader), sizeof(PublicHeader));
    _cleanupHeader(publicHeader);
    fileStream.seekg(publicHeader.headerSize);

    if (publicHeader.numberOfVariableLengthRecords > 0) {
      recordHeaders.resize(publicHeader.numberOfVariableLengthRecords);

      for (auto & header : recordHeaders) {
        fileStream.read(reinterpret_cast<char*>(&header), sizeof(RecordHeader::RAW_SIZE));
        uint16_t bytesToRead = header.recordLengthAfterHeader;
        header.data.reserve(bytesToRead);
        fileStream.read(header.data.data(), bytesToRead);
      }
    }

    _pointDataCount = publicHeader.legacyNumberOfPointRecords > 0
      ? publicHeader.legacyNumberOfPointRecords
      : publicHeader.numberOfPointRecords;

    fileStream.close();
  }

  template <typename T>
  uint64_t LASFile<T>::loadData(
    uint32_t minX,
    uint32_t maxX,
    uint32_t minY,
    uint32_t maxY,
    uint32_t minZ,
    uint32_t maxZ
  ) {
    std::ifstream fileStream(filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", filePath));
    }

    fileStream.seekg(publicHeader.offsetToPointData);

    uint64_t size = _loadData(
      publicHeader.pointDataRecordLength,
      fileStream,
      pointData,
      _pointDataCount,
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

  template <typename T>
  const uint64_t LASFile<T>::pointDataCount() const {
    return _pointDataCount;
  }

  template class LASFile<PointDataMin>;
}
