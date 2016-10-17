#pragma once
#ifndef LAS_RECORD_HEADER_HPP
#define LAS_RECORD_HEADER_HPP

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
    
    template<class T, size_t N>
    static void insertIntoArray(std::ifstream & in, std::array<T, N> & valueArray) {
      for (auto & value : valueArray) {
        in >> value;
      }
    }

    friend std::ifstream & operator>>(std::ifstream & in, RecordHeader & header) {
      in >> header.reserved;
      insertIntoArray(in, header.userID);
      in >> header.recordID;
      in >> header.recordLengthAfterHeader;
      insertIntoArray(in, header.description);
      return in;
    }

  };
}
#pragma pack(pop)

#endif	// LAS_RECORD_HEADER_HPP
