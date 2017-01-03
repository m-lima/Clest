#pragma once

#include <array>
#include <fstream>

using string = std::string;

#pragma pack(push, 1)
namespace las {
  struct RecordHeader {
    static constexpr size_t RAW_SIZE = 54;

    uint16_t reserved;
    std::array<char, 16> userID;
    uint16_t recordID;
    uint16_t recordLengthAfterHeader;
    std::array<char, 32> description;
    std::vector<char> data;
  };
}
#pragma pack(pop)
