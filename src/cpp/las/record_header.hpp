#pragma once

#include <array>

namespace las {
  struct RecordHeader {
    static constexpr size_t RAW_SIZE = 54;

#pragma pack(push, 1)
    uint16_t reserved;
    std::array<char, 16> userID;
    uint16_t recordID;
    uint16_t recordLengthAfterHeader;
    std::array<char, 32> description;
#pragma pack(pop)
    std::vector<char> data;
  };
}
