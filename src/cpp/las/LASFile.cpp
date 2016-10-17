#include <string>
#include <fstream>
#include <fmt/ostream.h>
#include <fmt/format.h>
#include "LASFile.hpp"

namespace {

  const std::string _simplifySize(uint64_t size) {
    if (size < 1000) {
      return fmt::format("{:d}B", size);
    }

    double output = size;

    output /= 1000.0;
    if (output < 1000) {
      return fmt::format("{:03.2f}kB", output);
    }
    
    output /= 1000.0;
    if (output < 1000) {
      return fmt::format("{:03.2f}MB", output);
    }
    
    output /= 1000.0;
    if (output < 1000) {
      return fmt::format("{:03.2f}GB", output);
    }

    output /= 1000.0;
    if (output < 1000) {
      return fmt::format("{:03.2f}TB", output);
    }

    return "<invalid>";
  }

  void _cleanupHeader(las::PublicHeader & header) {
    if (header.headerSize < sizeof(las::PublicHeader)) {
      char * rawBytes = reinterpret_cast<char*>(&header);
      for (int i = header.headerSize + 1; i < sizeof(las::PublicHeader); i++) {
        rawBytes[i] = 0;
      }
    }
  }

  template <uint32_t N, typename T>
  void _loadData(std::ifstream & in, std::list<T> & list, uint64_t max) {
    for (uint64_t i = 0; in.good();) {
      T data[N];
      in.read(reinterpret_cast<char*>(&data), sizeof(T) * N);
      for (auto & datum : data) {
        if (i++ < max) {
          list.push_back(datum);
        } else {
          return;
        }
      }
    }
  }
}

namespace las {
  LASFile::LASFile(const std::string & file)
    : _filePath(std::move(file)) {}

  void LASFile::loadHeaders() {
    std::ifstream fileStream(_filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", _filePath));
    } else {
      fmt::print("Loading LAS header from: {}\n", _filePath);
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

  void LASFile::loadData() {
    std::ifstream fileStream(_filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", _filePath));
    } else {
      fmt::print("Loading LAS data from: {}\n", _filePath);
    }

    fileStream.seekg(publicHeader.offsetToPointData);
    uint64_t pointDataCount = publicHeader.legacyNumberOfPointRecords > 0
      ? publicHeader.legacyNumberOfPointRecords
      : publicHeader.numberOfPointRecords;

    fmt::print("{:>5}: {}\n", "Try", pointDataCount);
    fmt::print("{:>5}: {}\n", "Size", _simplifySize(pointDataCount * sizeof(PointDataZero)));

    _loadData<4096>(fileStream, pointData, pointDataCount);
    fmt::print("{:>5}: {}\n", "Added", pointData.size());

    fileStream.close();
  }
}
