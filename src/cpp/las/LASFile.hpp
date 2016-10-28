#ifndef LAS_LAS_FILE_HPP
#define LAS_LAS_FILE_HPP

#include <vector>
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
    std::vector<T> pointData;

    const std::string filePath;

    void loadHeaders();
    uint64_t loadData(const Limits<uint32_t> & limits = Limits<uint32_t>());
    uint64_t pointDataCount() const;
    void save() const {
      save(filePath);
    }
    void save(std::string file) const;

  private:
    uint64_t _pointDataCount;
  };
}
#endif	// LAS_LAS_FILE_HPP
