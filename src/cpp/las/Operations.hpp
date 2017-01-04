#pragma once

#include "las_file.hpp"

namespace las {
  template <int N>
  void colorize(const LASFile<N> & lasFile);

  template <int N>
  void simplify(const LASFile<N> & lasFile, const double factor);

#ifdef CGAL_LINKED_WITH_TBB
  template <int N>
  void wlopParallel(const LASFile<N> & lasFile,
                    const double percentage,
                    const double radius,
                    const unsigned int iterations,
                    const bool uniform);
#endif
}
