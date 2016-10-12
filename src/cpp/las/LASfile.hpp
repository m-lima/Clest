#ifndef LASFILE_H
#define LASFILE_H

#include <string>
#include <fstream>
#include "LASheader.hpp"

namespace las {
  LASheader read(const std::string &file) {

    std::ifstream fileStream(file);
    if (!fileStream.is_open()) {
      throw std::runtime_error("Could not open file " + file);
    }

    LASheader header;
    fileStream >> header;
    return header;
  }
}
#endif	// LASFILE_H
