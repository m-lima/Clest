#ifndef LAS_LAS_FILE_HPP
#define LAS_LAS_FILE_HPP

#include <vector>
#include <list>
#include "PublicHeader.hpp"
#include "RecordHeader.hpp"
#include "PointData.hpp"

namespace las {
  struct LASFile {
    LASFile(const std::string & file);

    PublicHeader publicHeader;
    std::vector<RecordHeader> recordHeaders;
    std::list<PointDataZero> pointData;

    void loadHeaders();
    void loadData();

  private:
    const std::string _filePath;
  };
}
#endif	// LAS_LAS_FILE_HPP
