#ifndef LAS_LAS_FILE_HPP
#define LAS_LAS_FILE_HPP

#include <vector>
#include <deque>
#include "PublicHeader.hpp"
#include "RecordHeader.hpp"
#include "PointData.hpp"

namespace las {

  bool isLasValid(const PublicHeader & header);

  struct LASFile {
    LASFile(const std::string & file);

    PublicHeader publicHeader;
    std::vector<RecordHeader> recordHeaders;
    std::deque<PointDataMin> pointData;

    void loadHeaders();
    uint64_t loadAllData();
    uint64_t loadChunk(
      uint32_t minX,
      uint32_t maxX,
      uint32_t minY,
      uint32_t maxY,
      uint32_t minZ,
      uint32_t maxZ
    );

  private:
    const std::string _filePath;
  };
}
#endif	// LAS_LAS_FILE_HPP
