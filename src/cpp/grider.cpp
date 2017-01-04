#ifdef MODE_GRIDER

#include <clest/util.hpp>
#include <clest/ostream.hpp>

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
  auto loadPath = clest::getCmdOption(argv, argv + argc, "-l");
  if (loadPath) {
    grid::GridFile grid(loadPath);
    clest::println("Size: {} {} {}\nMax: {}",
                   grid.sizeX(),
                   grid.sizeY(),
                   grid.sizeZ(),
                   grid.maxValue());
  } else {
    auto convertPath = clest::getCmdOption(argv, argv + argc, "-c");
    if (convertPath) {

    }
  }

  return 0;
}

#endif