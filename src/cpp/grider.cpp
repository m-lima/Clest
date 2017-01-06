#include <iostream>

#include <clest/util.hpp>
#include <clest/ostream.hpp>

#include "las/grid_file.hpp"
#include "lewiner/MarchingCubes.h"
#include "mesh/cube_marcher.hpp"
#include "cl/cl_runner.hpp"

/// Create a grid based on a LASFile
/// It loads the proper `LASFile<N>` at compile time to speed up the loading,
/// since memory is not an issue given that the LASFile will be discarded
/// as soon as the grid is created
grid::GridFile convertGrid(const std::string & path,
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

int extractType(char * typeParam, char * convertPath) {
  int type = -1;
  if (typeParam) {
    try {
      type = std::stoi(typeParam);
      clest::println("Type specified as: Format{}", type);
    } catch (...) {
      clest::println(stderr, "Invalid type value [{}]\nUsing default -1");
    }
  } else {
    try {
      clest::println("No type given. Auto-detecting native type");
      las::LASFile<-1> dummyLas(convertPath);
      dummyLas.loadHeaders();
      type = dummyLas.publicHeader.pointDataRecordFormat;
      clest::println("Type auto-detected as: Format{}", type);
    } catch (...) {
      clest::println(stderr, "Invalid type value [{}]\nUsing default -1");
    }
  }

  return type;
}

unsigned short extractSize(char * sizeParam, char axis) {
  unsigned size = 256;
  if (sizeParam) {
    try {
      size = std::stoi(sizeParam);
    } catch (...) {
      if (size <= 0) {
        clest::println(stderr,
                       "Size of {} has to be greater than zero: {}\n"
                       "Reverting to 256",
                       axis,
                       sizeParam);
        size = 256;
      }
      clest::println(stderr,
                     "Invalid value for {} size: {}\nReverting to 256",
                     axis,
                     sizeParam);
      size = 256;
    }
  }
  clest::println("Using {} size of: {}", axis, size);
  return size;
}

grid::GridFile getGrid(int argc, char * argv[]) {
  // Load from an existing grid
  if (clest::findOption(argv, argv + argc, "-l")) {
    clest::println("Load flag found. Ignoring all conversion flags");

    auto loadPath = clest::extractOption(argv, argv + argc, "-l");
    if (loadPath) {
      clest::println("Loading existing grid from:\n"
                     "{}\n",
                     loadPath);

      return loadGrid(loadPath);

    // Load switch was used, but no file was given
    } else {
      clest::println(stderr, "Error: No file path was given. Quitting..");
      std::quick_exit(-1);
    }

  // Load from an existing LAS
  } else if (clest::findOption(argv, argv + argc, "-c")) {

    auto convertPath = clest::extractOption(argv, argv + argc, "-c");
    if (convertPath) {
      clest::println("Converting into a grid from:\n{}", convertPath);

      auto typeParam = clest::extractOption(argv, argv + argc, "-t");
      auto xParam = clest::extractOption(argv, argv + argc, "-x");
      auto yParam = clest::extractOption(argv, argv + argc, "-y");
      auto zParam = clest::extractOption(argv, argv + argc, "-z");

      int type = extractType(typeParam, convertPath);
      unsigned short sizeX = extractSize(xParam, 'X');
      unsigned short sizeY = extractSize(yParam, 'Y');
      unsigned short sizeZ = extractSize(zParam, 'Z');

      clest::println("Creating grid..");

      auto grid = convertGrid(convertPath, type, sizeX, sizeY, sizeZ);

      // Save grid
      if (clest::findOption(argv, argv + argc, "-s")) {
        clest::println("Saving grid");

        auto savePath = clest::extractOption(argv, argv + argc, "-s");
        if (savePath && savePath[0] != '-') {
          clest::println("Saving as the given name:\n{}", savePath);
          grid.save(savePath);
        } else {
          savePath = clest::extractOption(argv, argv + argc, "-l");
          if (!savePath) {
            savePath = clest::extractOption(argv, argv + argc, "-c");
          }

          clest::println("Saving as the automatic name:\n{}.grid", savePath);
          grid.save(fmt::format("{}.grid", savePath));
        }
      }

      return grid;

    // Covert switch was used, but no file was given
    } else {
      clest::println(stderr, "Error: No file path was given. Quitting..");
      std::quick_exit(-1);
    }

    // None of the switches were found
  } else {
    clest::println(stderr, "No valid parameters given.. Nothing to do");
    std::quick_exit(-1);
  }
}

void performMarchingCubes(grid::GridFile & grid,
                          const char * const path,
                          float threshold) {
  MarchingCubes marchingCubes;
  marchingCubes.set_resolution(grid.sizeX(), grid.sizeY(), grid.sizeZ());
  marchingCubes.init_all();
  clest::println("MC: [{} {} {}]",
                 marchingCubes.size_x(),
                 marchingCubes.size_y(),
                 marchingCubes.size_z());

  for (int i = 0; i < grid.sizeX(); ++i) {
    for (int j = 0; j < grid.sizeY(); ++j) {
      for (int k = 0; k < grid.sizeZ(); ++k) {
        marchingCubes.set_data(grid.data(i, j, k), i, j, k);
      }
    }
  }
  marchingCubes.run(grid.maxValue() * threshold);

  marchingCubes.clean_temps();
  marchingCubes.writePLY(path);

  marchingCubes.clean_all();
}

int main(int argc, char * argv[]) {

  clest::println("== Parameters ===============");
  for (int i = 1; i < argc; ++i) {
    clest::println(argv[i]);
  }
  clest::println("=============================");
  
  clest::ClRunner::printFull();
  clest::ClRunner ocl(CL_DEVICE_TYPE_GPU, { "fp64" });
  ocl.loadProgram("marching", "opencl/marching.cl");
  ocl.makeKernel<cl::Buffer>("marching", "createGrid");

  auto grid = getGrid(argc, argv);

  // March grid
  if (clest::findOption(argv, argv + argc, "-m")) {
    clest::println("Marching the grid..");

    auto dividerParam = clest::extractOption(argv, argv + argc, "-d");
    float divider = 0.5f;
    if (dividerParam) {
      try {
        int dividerPercentage = std::stoi(dividerParam);
        if (dividerPercentage <= 0 || dividerPercentage >= 100) {
          clest::println(stderr,
                         "The divider parameters must be an integer "
                         "greater than 0% and less than 100%\n"
                         "Reverting to 50%");
        } else {
          divider = dividerPercentage / 100.0f;
        }
      } catch (...) {
        clest::println(stderr,
                       "The divider parameters must be an integer "
                       "greater than 0% and less than 100%\n"
                       "Reverting to 50%");
      }
    }

    auto savePath = clest::extractOption(argv, argv + argc, "-m");
    if (savePath && savePath[0] != '-') {
      clest::println("Saving the mesh as the given name:\n{}", savePath);
      performMarchingCubes(grid, savePath, divider);
    } else {
      savePath = clest::extractOption(argv, argv + argc, "-l");
      if (!savePath) {
        savePath = clest::extractOption(argv, argv + argc, "-c");
      }

      clest::println("Saving the mesh as the automatic name:\n{}.ply",
                     savePath);
      performMarchingCubes(grid,
                           fmt::format("{}.ply", savePath).c_str(),
                           divider);
    }
  }

  clest::println("Done!");
  std::cin.get();
  return 0;
}

//TODO: Test command-line args
