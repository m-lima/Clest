#include <iostream>
#include <chrono>

#include <clest/util.hpp>
#include <clest/ostream.hpp>

#include "las/grid_file.hpp"
#include "cl/cl_runner.hpp"
#include "mesher.hpp"

void printUsage() {
  clest::println(
    "Usage:\n"
    "clest LOADING [-t threshold] [-g | -s gridOutput] [-m [meshOutput]]\n"
    "LOADING:\n"
    "\t-l gridFile:\n"
    "\t\tLoad grid from file gridFile\n"
    "\t-c LASFile [-x size] [-y size] [-z size]:"
    "\t\tConvert LAS from file LASFiles and specify the size of the converted grid"
  );
}

template <int N>
void load(const std::string & path) {
  las::LASFile<N> lasTest(path);
  lasTest.loadHeaders();
  auto start = std::chrono::high_resolution_clock::now();
  lasTest.loadData();
  auto end = std::chrono::high_resolution_clock::now();
  clest::println("{}",
                 std::chrono::duration_cast<std::chrono::nanoseconds>
                 (end - start).count());
}

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

template<clest::MesherDevice D>
clest::Mesher<D> loadGrid(const char * loadPath) {
  clest::println("Load flag found. Ignoring all conversion flags");

  if (loadPath) {
    clest::println("Loading existing grid from:\n"
                   "{}\n",
                   loadPath);

    try {
      clest::println();
      return clest::Mesher<D>(loadPath);
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
}

template<clest::MesherDevice D>
clest::Mesher<D> convertLas(const std::vector<const char*> & args) {
  auto convertPath = clest::extractOption(args, "-c");
  if (convertPath) {
    clest::println("Converting into a grid from:\n{}", convertPath);
    auto convertPath = clest::extractOption(args, "-c");

    auto xParam = clest::extractOption(args, "-x");
    auto yParam = clest::extractOption(args, "-y");
    auto zParam = clest::extractOption(args, "-z");

    unsigned short sizeX = extractSize(xParam, 'X');
    unsigned short sizeY = extractSize(yParam, 'Y');
    unsigned short sizeZ = extractSize(zParam, 'Z');

    return clest::Mesher<D>(convertPath, sizeX, sizeY, sizeZ);
  } else {
    clest::println(stderr, "Error: No file path was given. Quitting..");
    std::exit(-1);
  }
}

template<clest::MesherDevice D>
clest::Mesher<D> getMesher(const std::vector<const char*> & args) {
  // Load from an existing grid
  if (clest::findOption(args, "-l")) {
    auto loadPath = clest::extractOption(args, "-l");
    return loadGrid<D>(loadPath);
  }

  if (clest::findOption(args, "-c")) {
    return convertLas<D>(args);
  }

  clest::println(stderr, "No valid parameters given.. Nothing to do");
  std::exit(-1);
}

void saveGrid(const clest::Mesher<clest::GPU_DEVICE> & mesher,
              const std::vector<const char*> & args) {
  clest::println(stderr,
                 "Cannot save grid when converting in the GPU "
                 "(using -g)\nIgnoring save flag..");
  clest::println();
}

void saveGrid(const clest::Mesher<clest::CPU_DEVICE> & mesher,
              const std::vector<const char*> & args) {
  clest::println("Saving grid");

  auto savePath = clest::extractOption(args, "-s");
  if (savePath && savePath[0] != '-') {
    clest::println("Saving as the given name:\n{}", savePath);
    mesher.grid().save(savePath);
  } else {
    savePath = clest::extractOption(args, "-l");
    if (!savePath) {
      savePath = clest::extractOption(args, "-c");
    }

    clest::println("Saving as the automatic name:\n{}.grid", savePath);
    mesher.grid().save(fmt::format("{}.grid", savePath));
  }
}

template<clest::MesherDevice D>
void marchGrid(const clest::Mesher<D> & mesher,
               const std::vector<const char*> & args) {
  if (clest::findOption(args, "-m")) {
    clest::println("Marching the grid..");

    auto dividerParam = clest::extractOption(args, "-t");
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

    auto savePath = clest::extractOption(args, "-m");
    if (savePath && savePath[0] != '-') {
      clest::println("Saving the mesh as the given name:\n{}", savePath);
      mesher.performMarchingCubes(savePath, divider);
    } else {
      savePath = clest::extractOption(args, "-l");
      if (!savePath) {
        savePath = clest::extractOption(args, "-c");
      }

      clest::println("Saving the mesh as the automatic name:\n{}.ply",
                     savePath);
      mesher.performMarchingCubes(fmt::format("{}.ply", savePath).c_str(),
                                  divider);
    }
  }
}

template<clest::MesherDevice D>
int templatedMain(const std::vector<const char*> & args) {
  auto mesher = getMesher<D>(args);

  if (clest::findOption(args, "-s")) {
    saveGrid(mesher, args);
  }

  marchGrid(mesher, args);

  clest::println("Done!");
  std::cin.get();
  return 0;
}

int main(int argc, char * argv[]) {

  std::vector<const char*> args(argv + 1, argv + argc);
    
  clest::ClRunner::printFull();
  clest::println();

  if (clest::findOption(args, "-g")) {
    return templatedMain<clest::GPU_DEVICE>(args);
  } else {
    return templatedMain<clest::CPU_DEVICE>(args);
  }
}
