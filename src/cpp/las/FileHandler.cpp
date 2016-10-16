#include <string>
#include <fstream>
#include <iostream>
#include "FileHandler.hpp"

namespace {
  void cleanupHeader(las::PublicHeader & header) {
    if (header.headerSize < sizeof(las::PublicHeader)) {
      char * rawBytes = reinterpret_cast<char*>(&header);
      for (int i = header.headerSize + 1; i < sizeof(las::PublicHeader); i++) {
        rawBytes[i] = 0;
      }
    }
  }

  template <typename T, uint32_t N>
  void loadData(std::ifstream & in, std::list<T> & list, uint64_t max) {
    for (uint64_t i = 0; in.good();) {
      T data[N];
      in.read(reinterpret_cast<char*>(&data), sizeof(T) * N);
      for (auto & datum : data) {
        if (i++ < max) {
          list.push_back(datum);
        } else {
          std::cout << "Add: " << i << std::endl;
          return;
        }
      }
    }
    std::cout << "Add: " << list.size() << std::endl;
  }
}

namespace las {
  LASFile read(const std::string &file) {

    std::ifstream fileStream(file, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error("Could not open file " + file);
    }

    LASFile output;
    fileStream.read(reinterpret_cast<char*>(&output.publicHeader), sizeof(PublicHeader));
    cleanupHeader(output.publicHeader);
    fileStream.seekg(output.publicHeader.headerSize);

    if (output.publicHeader.numberOfVariableLengthRecords > 0) {
      output.recordHeaders.resize(output.publicHeader.numberOfVariableLengthRecords);

      for (auto & header : output.recordHeaders) {
        fileStream.read(reinterpret_cast<char*>(&header), sizeof(RecordHeader));
      }
    }

    fileStream.seekg(output.publicHeader.offsetToPointData);
    uint64_t pointDataCount = output.publicHeader.legacyNumberOfPointRecords > 0
      ? output.publicHeader.legacyNumberOfPointRecords
      : output.publicHeader.numberOfPointRecords;

    std::cout << "Max: " << output.pointData.max_size() << std::endl;
    std::cout << "Try: " << pointDataCount << std::endl;
    std::cout << "Tot: " << pointDataCount * sizeof(PointDataZero) << std::endl;
    //output.pointData.resize(pointDataCount);

    loadData<PointDataZero, 4096>(fileStream, output.pointData, pointDataCount);

    fileStream.close();
    return output;
  }
}
