#include <string>
#include <fstream>
#include <fmt/ostream.h>
#include "LASFile.hpp"

namespace {
  constexpr uint32_t BUFFER_SIZE = 65536;

  size_t _sizeOfFormat(uint8_t format) {
    if (format == 0) return sizeof(las::PointDataZero);
    if (format == 1) return sizeof(las::PointDataOne);
    if (format == 2) return sizeof(las::PointDataTwo);
    if (format == 3) return sizeof(las::PointDataThree);
    if (format == 4) return sizeof(las::PointDataFour);
    if (format == 5) return sizeof(las::PointDataFive);
    throw std::invalid_argument(fmt::format("Format {} is not recognized"));
  }

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
    size_t typeSize,
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

    char data[BUFFER_SIZE];
    if (
      minX == -1 &&
      maxX == 0 &&
      minY == -1 &&
      maxY == 0 &&
      minZ == -1 &&
      maxZ == 0) {
      while (count < max && in.good()) {
        in.read(data, BUFFER_SIZE);
        for (size_t i = typeSize; i < BUFFER_SIZE; i += typeSize) {
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
        in.read(data, BUFFER_SIZE);
        for (size_t i = typeSize; i < BUFFER_SIZE; i += typeSize) {
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

      char buffer[0xFFFF];
      for (auto & header : recordHeaders) {
        fileStream.read(reinterpret_cast<char*>(&header), sizeof(RecordHeader::RAW_SIZE));
        fileStream.read(buffer, header.recordLengthAfterHeader);
        header.data.reserve(header.recordLengthAfterHeader);
        std::memcpy(buffer, header.data.data(), header.recordLengthAfterHeader);
      }
    }

    fileStream.seekg(publicHeader.offsetToPointData);
    _pointDataCount = publicHeader.legacyNumberOfPointRecords > 0
      ? publicHeader.legacyNumberOfPointRecords
      : publicHeader.numberOfPointRecords;
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
      _sizeOfFormat(publicHeader.pointDataRecordFormat),
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
