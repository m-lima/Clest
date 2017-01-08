#pragma once

#include <vector>
#include "public_header.hpp"
#include "record_header.hpp"
#include "point_data.hpp"

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
    bool isValidAndLoaded() const;
    bool isValidAndFullyLoaded() const;
    void loadHeaders();
    uint64_t loadData(const Limits<uint32_t> & limits = Limits<uint32_t>());
    uint64_t pointDataCount() const;
    
    void save(std::string file) const;
    
    void releaseRecordHeaders() {
      recordHeaders = std::vector<RecordHeader>(0);
    };
      
    void releaseData() {
      pointData = std::vector<PointData<N>>(0);
    };
    
    void save() const {
      save(filePath);
    }

  private:
    uint64_t _pointDataCount;
  };
}
