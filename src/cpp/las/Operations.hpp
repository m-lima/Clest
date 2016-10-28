#ifndef LAS_OPERATIONS_HPP
#define LAS_OPERATIONS_HPP

#include "LASFile.hpp"

namespace las {
  template <typename T>
  void colorize(const LASFile<T> & lasFile);

  template <typename T>
  void simplify(const LASFile<T> & lasFile, const double factor);

#ifdef CGAL_LINKED_WITH_TBB
  template <typename T>
  void wlopParallel(const LASFile<T> & lasFile);
#endif
}
#endif // LAS_OPERATIONS_HPP