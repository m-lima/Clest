#include <iostream>

#include <clest/util.hpp>
#include <clest/ostream.hpp>

#include "las/grid_file.hpp"
#include "lewiner/MarchingCubes.h"
#include "mesh/cube_marcher.hpp"
#include "cl/cl_runner.hpp"
#include "mesher.hpp"

unsigned short extractSize(const char * sizeParam, const char axis) {
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

clest::Mesher getMesher(const std::vector<const char*> args) {
  // Load from an existing grid
  if (clest::findOption(args, "-l")) {
    clest::println("Load flag found. Ignoring all conversion flags");

    auto loadPath = clest::extractOption(args, "-l");
    if (loadPath) {
      clest::println("Loading existing grid from:\n"
                     "{}\n",
                     loadPath);

      try {
        clest::println();
        return clest::Mesher(grid::GridFile(loadPath));
      } catch (...) {
        clest::println(stderr,
                       "The application could not proceed and is quitting");
        std::exit(-1);
      }

    // Load switch was used, but no file was given
    } else {
      clest::println(stderr, "Error: No file path was given. Quitting..");
      std::exit(-1);
    }

  // Load from an existing LAS
  } else if (clest::findOption(args, "-c")) {

    auto convertPath = clest::extractOption(args, "-c");
    if (convertPath) {
      clest::println("Converting into a grid from:\n{}", convertPath);

      //auto typeParam = clest::extractOption(args, "-t");
      auto xParam = clest::extractOption(args, "-x");
      auto yParam = clest::extractOption(args, "-y");
      auto zParam = clest::extractOption(args, "-z");

      //int type = extractType(typeParam, convertPath);
      unsigned short sizeX = extractSize(xParam, 'X');
      unsigned short sizeY = extractSize(yParam, 'Y');
      unsigned short sizeZ = extractSize(zParam, 'Z');

      if (clest::findOption(args, "-g")) {
        if (clest::findOption(args, "-s")) {
          clest::println(stderr, "Cannot save grid when converting in the GPU "
                         "(using -g)\nIgnoring save flag..");
        }
        clest::println();
        return clest::Mesher(convertPath, sizeX, sizeY, sizeZ);
      }

      clest::println("Creating grid..");

      auto grid = grid::GridFile(las::LASFile<-2>(convertPath),
                                 sizeX,
                                 sizeY,
                                 sizeZ);
        //convertGrid(convertPath, type, sizeX, sizeY, sizeZ);

      // Save grid
      if (clest::findOption(args, "-s")) {
        clest::println("Saving grid");

        auto savePath = clest::extractOption(args, "-s");
        if (savePath && savePath[0] != '-') {
          clest::println("Saving as the given name:\n{}", savePath);
          grid.save(savePath);
        } else {
          savePath = clest::extractOption(args, "-l");
          if (!savePath) {
            savePath = clest::extractOption(args, "-c");
          }

          clest::println("Saving as the automatic name:\n{}.grid", savePath);
          grid.save(fmt::format("{}.grid", savePath));
        }
      }

      return clest::Mesher(grid);

    // Covert switch was used, but no file was given
    } else {
      clest::println(stderr, "Error: No file path was given. Quitting..");
      std::exit(-1);
    }

    // None of the switches were found
  } else {
    clest::println(stderr, "No valid parameters given.. Nothing to do");
    std::exit(-1);
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

  std::vector<const char*> args(argc - 1);

  clest::println("== Parameters ===============");
  for (int i = 1; i < argc; ++i) {
    clest::println(argv[i]);
    args[i - 1] = argv[i];
  }
  clest::println("=============================");
  clest::println();
  
  clest::ClRunner::printFull();
  clest::println();

  auto mesh = getMesher(args);

  //// March grid
  //if (clest::findOption(args, "-m")) {
  //  clest::println("Marching the grid..");

  //  auto dividerParam = clest::extractOption(args, "-d");
  //  float divider = 0.5f;
  //  if (dividerParam) {
  //    try {
  //      int dividerPercentage = std::stoi(dividerParam);
  //      if (dividerPercentage <= 0 || dividerPercentage >= 100) {
  //        clest::println(stderr,
  //                       "The divider parameters must be an integer "
  //                       "greater than 0% and less than 100%\n"
  //                       "Reverting to 50%");
  //      } else {
  //        divider = dividerPercentage / 100.0f;
  //      }
  //    } catch (...) {
  //      clest::println(stderr,
  //                     "The divider parameters must be an integer "
  //                     "greater than 0% and less than 100%\n"
  //                     "Reverting to 50%");
  //    }
  //  }

  //  auto savePath = clest::extractOption(args, "-m");
  //  if (savePath && savePath[0] != '-') {
  //    clest::println("Saving the mesh as the given name:\n{}", savePath);
  //    performMarchingCubes(grid, savePath, divider);
  //  } else {
  //    savePath = clest::extractOption(args, "-l");
  //    if (!savePath) {
  //      savePath = clest::extractOption(args, "-c");
  //    }

  //    clest::println("Saving the mesh as the automatic name:\n{}.ply",
  //                   savePath);
  //    performMarchingCubes(grid,
  //                         fmt::format("{}.ply", savePath).c_str(),
  //                         divider);
  //  }
  //}

  clest::println("Done!");
  std::cin.get();
  return 0;
}

//TODO: Test command-line args
