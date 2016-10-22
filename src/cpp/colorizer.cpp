#include <fmt/ostream.h>
#include "las/LASFile.hpp"

using namespace las;

namespace clest {
  //void colorize(const LASFile & lasFile) {
  //  if (!isLasValid(lasFile.publicHeader)) {
  //    fmt::print(stderr, "Trying to colorize LAS file, but {} seems to be corrupted.\n", lasFile.filePath);
  //  }

  //  if (lasFile.pointDataCount() < 1) {
  //    fmt::print(stderr, "Trying to colorize LAS file, but {} seems to be empty.\n", lasFile.filePath);
  //  }

  //  //LASFile newFile(lasFile.filePath + ".color.las");

  //  //newFile.publicHeader = lasFile.publicHeader;
  //  //std::copy(lasFile.recordHeaders.cbegin(), lasFile.recordHeaders.cend(), newFile.recordHeaders);
  //}
}