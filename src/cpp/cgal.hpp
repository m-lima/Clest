#ifndef CLEST_CGAL_HPP
#define CLEST_CGAL_HPP

#include <fmt/ostream.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

namespace {

}

namespace clest {
#ifdef CGAL_LINKED_WITH_TBB
  void wlopParallel() {
  }
#endif
  void wlopSequential() {
    CGAL::Simple_cartesian<double>::Point_3 point;
  }
}
#endif // CLEST_CGAL_HPP