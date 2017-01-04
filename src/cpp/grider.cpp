#ifdef MODE_GRIDER

#include <fmt/ostream.h>

#include "las/GridFile.hpp"

template <int N = -1>
void createGrid(const std::string & path) {
  las::LASFile<N> las(fmt::format("{}.las", path));
  las.loadHeaders();
  las.loadData();

  grid::GridFile grid(las, 256, 256, 256);
  grid.save(fmt::format("{}.grid", path));
}

int main(int argc, char * argv[]) {
  grid::GridFile grid(argv[1]);
  fmt::print("Size: {} {} {}\nMax: {}\n",
             grid.sizeX(),
             grid.sizeY(),
             grid.sizeZ(),
             grid.maxValue());

  return 0;
}

#endif