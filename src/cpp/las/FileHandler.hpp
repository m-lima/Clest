#ifndef LAS_FILE_HANDLER_HPP
#define LAS_FILE_HANDLER_HPP

#include <vector>
#include <list>
#include "PublicHeader.hpp"
#include "RecordHeader.hpp"
#include "PointData.hpp"

namespace las {
  struct LASFile {
    PublicHeader publicHeader;
    std::vector<RecordHeader> recordHeaders;
    std::list<PointDataZero> pointData;
  };

  LASFile read(const std::string &file);
}
#endif	// LAS_FILE_HANDLER_HPP
