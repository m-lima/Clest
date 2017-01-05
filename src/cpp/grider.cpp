#include <clest/util.hpp>
#include <clest/ostream.hpp>

#include "las/grid_file.hpp"

/// Create a grid based on a LASFile
/// It loads the proper `LASFile<N>` at compile time to speed up the loading,
/// since memory is not an issue given that the LASFile will be discarded
/// as soon as the grid is created
grid::GridFile createGrid(const std::string & path,
                          int type,
                          unsigned short sizeX,
                          unsigned short sizeY,
                          unsigned short sizeZ) noexcept {
#define __DECLARE_TEMPLATES(index)\
    case index:\
    {\
      las::LASFile<index> las(path);\
      return grid::GridFile(las, sizeX, sizeY, sizeZ);\
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

  // Load from an existing grid
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

    // Load switch was used, but no file was given
    } else {
      clest::println(stderr, "Error: No file path was given. Quitting..");
    }

  // Load from an existing LAS
  } else if (clest::findOption(argv, argv + argc, "-l")) {

    auto convertPath = clest::extractOption(argv, argv + argc, "-c");
    if (convertPath) {
      clest::println("Converting into a grid from:\n{}", convertPath);

      auto typeParam = clest::extractOption(argv, argv + argc, "-t");
      auto xParam = clest::extractOption(argv, argv + argc, "-x");
      auto yParam = clest::extractOption(argv, argv + argc, "-y");
      auto zParam = clest::extractOption(argv, argv + argc, "-z");

      // Preparing default values for grid conversion
      int type = -1;
      unsigned short sizeX = 256;
      unsigned short sizeY = 256;
      unsigned short sizeZ = 256;

      if (typeParam) {
        try {
          type = std::stoi(typeParam);
          clest::println("Type specified as: {}", type);
        } catch (...) {
          clest::println(stderr, "Invalid type value [{}]\mUsing default -1");
        }        
      } else {
        clest::println("No type given. Auto-detecting native type");
        las::LASFile<-1> dummyLas(convertPath);
        dummyLas.loadHeaders();
        type = dummyLas.publicHeader.pointDataRecordFormat;
        clest::println("Type auto-detected as: {}", type);
      }



      auto grid = createGrid(convertPath, type);

    // Covert switch was used, but no file was given
    } else {
      clest::println(stderr, "Error: No file path was given. Quitting..");
    }

  // None of the switches were found
  } else { 
    clest::println(stderr, "No valid parameters given.. Nothing to do");
  }

  return 0;
}

//TODO: Test command-line args
