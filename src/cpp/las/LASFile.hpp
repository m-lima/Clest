#pragma once

#include <vector>
#include "PublicHeader.hpp"
#include "RecordHeader.hpp"
#include "PointData.hpp"

namespace las {

  template <int N>
  class LASFile {
  public:
    LASFile(const std::string & file);

    PublicHeader publicHeader;
    std::vector<RecordHeader> recordHeaders;
    std::vector<PointData<N>> pointData;

    const std::string filePath;

    bool isValid() const;
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
