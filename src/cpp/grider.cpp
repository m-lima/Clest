//#ifdef MODE_GRIDER

#include <clest/util.hpp>
#include <clest/ostream.hpp>

#include "las/grid_file.hpp"

grid::GridFile createGrid(const std::string & path, int type) noexcept {
#define __DECLARE_TEMPLATES(index)\
    case index:\
    {\
      las::LASFile<index> las(path);\
      return grid::GridFile(las, 256, 256, 256);\
    }

  try {
    switch (type) {
      default:
        __DECLARE_TEMPLATES(-1)
          __DECLARE_TEMPLATES(0)
          __DECLARE_TEMPLATES(1)
          __DECLARE_TEMPLATES(2)
          __DECLARE_TEMPLATES(3)
    }
  } catch (...) {
    clest::println(stderr,
                   "The application could not proceed and is quitting");
    std::quick_exit(-1);
  }

#undef __DECLARE_TEMPLATES
}

grid::GridFile loadGrid(const std::string & path) noexcept {
  try {
    return grid::GridFile(path);
  } catch (...) {
    clest::println(stderr,
                   "The application could not proceed and is quitting");
    std::quick_exit(-1);
  }
}

int main(int argc, char * argv[]) {
  if (clest::findOption(argv, argv + argc, "-l")) {
    clest::println("Load flag found. Ignoring all other flags");

    auto loadPath = clest::extractOption(argv, argv + argc, "-l");

    if (loadPath) {
      clest::println("Loading existing grid from:\n"
                     "{}\n",
                     loadPath);

      auto grid = loadGrid(loadPath);

      clest::println("Size: {} {} {}\nMax: {}",
                     grid.sizeX(),
                     grid.sizeY(),
                     grid.sizeZ(),
                     grid.maxValue());
    }
  } else {
    auto convertPath = clest::extractOption(argv, argv + argc, "-c");
    if (convertPath) {
      clest::println("Converting into a grid from:\n{}", convertPath);
      auto typeParam = clest::extractOption(argv, argv + argc, "-t");
      int type;
      if (typeParam) {
        fmt::print("Type specified as: ");
        type = std::atoi(typeParam);
      } else {
        fmt::print("Type auto-detected as: ");
        las::LASFile<-1> dummyLas(convertPath);
        dummyLas.loadHeaders();
        type = dummyLas.publicHeader.pointDataRecordFormat;
      }

      clest::println("{}", type);
      auto grid = createGrid(convertPath, type);
    }
  }

  return 0;
}

//TODO: Test command-line args

//#endif