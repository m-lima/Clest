#include "cube_marcher.hpp"

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

namespace grid {

  void CubeMarcher::generateIntersections(uint32_t iso) {
    auto length = mGrid->cDataRaw()->size();
    tbb::parallel_for(size_t(0), length, [=](size_t i) {
      mGrid->cDataRaw[i];
    });
  }

  void CubeMarcher::march(uint32_t iso) {
    generateIntersections(iso);
  }

  void CubeMarcher::save(const std::string & name) {}

}
