#ifndef LAS_LAS_FILE_HPP
#define LAS_LAS_FILE_HPP

#include <vector>
#include <deque>
#include "PublicHeader.hpp"
#include "RecordHeader.hpp"
#include "PointData.hpp"

namespace las {

  bool isLasValid(const PublicHeader & header);

  template <typename T>
  class LASFile {
  public:
    LASFile(const std::string & file);

    PublicHeader publicHeader;
    std::vector<RecordHeader> recordHeaders;
    std::deque<T> pointData;

    const std::string filePath;

    void loadHeaders();
    uint64_t loadData(
      uint32_t minX = -1,
      uint32_t maxX = 0,
      uint32_t minY = -1,
      uint32_t maxY = 0,
      uint32_t minZ = -1,
      uint32_t maxZ = 0
    );
    const uint64_t pointDataCount() const;

  private:
    uint64_t _pointDataCount;
  };
}
#endif	// LAS_LAS_FILE_HPP