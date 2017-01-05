#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <clest/util.hpp>

#include "las/las_file.hpp"
#include "las/las_operations.hpp"

#ifdef _WIN32
// Force high performance GPU
extern "C" {
  // NVidia
  __declspec(dllexport) unsigned int NvOptimusEnablement = 1;

  // AMD
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace {

  template <int N>
  void _executeLoadChunks(las::LASFile<N> & lasFile) {
    uint32_t minX = static_cast<uint32_t>(
      (lasFile.publicHeader.minX - lasFile.publicHeader.xOffset)
      / lasFile.publicHeader.xScaleFactor);
    uint32_t maxX = static_cast<uint32_t>(
      (lasFile.publicHeader.maxX - lasFile.publicHeader.xOffset)
      / lasFile.publicHeader.xScaleFactor);
    uint32_t minY = static_cast<uint32_t>(
      (lasFile.publicHeader.minY - lasFile.publicHeader.yOffset)
      / lasFile.publicHeader.yScaleFactor);
    uint32_t maxY = static_cast<uint32_t>(
      (lasFile.publicHeader.maxY - lasFile.publicHeader.yOffset)
      / lasFile.publicHeader.yScaleFactor);
    uint32_t minZ = static_cast<uint32_t>(
      (lasFile.publicHeader.minZ - lasFile.publicHeader.zOffset)
      / lasFile.publicHeader.zScaleFactor);
    uint32_t maxZ = static_cast<uint32_t>(
      (lasFile.publicHeader.maxZ - lasFile.publicHeader.zOffset)
      / lasFile.publicHeader.zScaleFactor);

    constexpr unsigned int FACTOR = 2;

    uint32_t deltaX = (maxX + FACTOR - minX) / FACTOR;
    uint32_t deltaY = (maxY + FACTOR - minY) / FACTOR;
    uint32_t deltaZ = (maxZ + FACTOR - minZ) / FACTOR;

    uint64_t count = 0;

    for (int i = 0; i < FACTOR; i++) {
      uint32_t x = minX + deltaX * i;
      for (int j = 0; j < FACTOR; j++) {
        uint32_t y = minY + deltaY * j;
        for (int k = 0; k < FACTOR; k++) {
          uint32_t z = minZ + deltaZ * k;
          count += lasFile.loadData(Limits<uint32_t>(x,
                                                     x + deltaX,
                                                     y,
                                                     y + deltaY,
                                                     z,
                                                     z + deltaZ));
          fmt::print("Total: {}\n", count);
          fmt::print(" Step: {}/{}\n\n",
                     k + (FACTOR * j) + (FACTOR * FACTOR * i) + 1,
                     FACTOR * FACTOR * FACTOR);
        }
      }
    }

    fmt::print("File total: {}\n", lasFile.pointDataCount());
    fmt::print("Loaded total: {}\n\n", lasFile.pointData.size());
  }

  template <int N>
  void _executeLoadAll(las::LASFile<N> & lasFile) {
    boost::posix_time::ptime start =
      boost::posix_time::second_clock::local_time();
    fmt::print("Load All Starting [{}]\n",
               boost::posix_time::to_simple_string(start));
    fmt::print("Loading {} points\n",
               clest::simplifyValue(
                 static_cast<double>(lasFile.pointDataCount())));
    lasFile.loadData();
    fmt::print("Loaded {} points\n\n",
               clest::simplifyValue(
                 static_cast<double>(lasFile.pointData.size())));
    boost::posix_time::ptime end =
      boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration duration = end - start;
    fmt::print("Load All Finished [{}]\n",
               boost::posix_time::to_simple_string(end));
    fmt::print("Load All Duration [{}]\n\n",
               boost::posix_time::to_simple_string(duration));
  }

  template <int N>
  void _executeSimplify(const las::LASFile<N> & lasFile, const double factor) {
    boost::posix_time::ptime start =
      boost::posix_time::second_clock::local_time();
    fmt::print("Simplify Starting [{}]\n",
               boost::posix_time::to_simple_string(start));
    fmt::print("Parameters:\nNumber of points: {}\nPercentage to keep: {}%\n",
               lasFile.pointDataCount(),
               factor);
    las::simplify(lasFile, factor);
    boost::posix_time::ptime end =
      boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration duration = end - start;
    fmt::print("Simplify Starting Finished [{}]\n",
               boost::posix_time::to_simple_string(end));
    fmt::print("Simplify Starting Duration [{}]\n\n",
               boost::posix_time::to_simple_string(duration));
  }

  template <int N>
  void _executeColorize(const las::LASFile<N> & lasFile) {
    boost::posix_time::ptime start =
      boost::posix_time::second_clock::local_time();
    fmt::print("Colorize Starting [{}]\n",
               boost::posix_time::to_simple_string(start));
    las::colorize(lasFile);
    boost::posix_time::ptime end =
      boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration duration = end - start;
    fmt::print("Colorize Finished [{}]\n",
               boost::posix_time::to_simple_string(end));
    fmt::print("Colorize Duration [{}]\n\n",
               boost::posix_time::to_simple_string(duration));
  }

  template <int N>
  void _executeCGALWLOP(const las::LASFile<N> & lasFile,
                       const double percentage,
                       const double radius,
                       const unsigned int iterations,
                       const bool uniform) {
    boost::posix_time::ptime start =
      boost::posix_time::second_clock::local_time();
    fmt::print("CGAL WLOP Starting [{}]\n",
               boost::posix_time::to_simple_string(start));
    fmt::print("Parameters:\n"
               "Number of points: {}\n"
               "Percentage to keep: {}%\n"
               "Radius: {}\n"
               "Number of iterations: {}\n"
               "Requires uniform: {}\n\n",
               lasFile.pointDataCount(),
               percentage,
               radius,
               iterations,
               uniform);
    las::wlopParallel(lasFile, percentage, radius, iterations, uniform);
    boost::posix_time::ptime end =
      boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration duration = end - start;
    fmt::print("CGAL WLOP Finished [{}]\n",
               boost::posix_time::to_simple_string(end));
    fmt::print("CGAL WLOP Duration [{}]\n\n",
               boost::posix_time::to_simple_string(duration));
  }

  template <int N>
  int _mainExecuteBlock(las::LASFile<N> & lasFile) {
    int returnValue = 0;

    lasFile.loadHeaders();

    _executeLoadAll(lasFile);
    //_executeLoadChunks(lasFile);
    //_executeSimplify(lasFile, 25);
    //_executeColorize(lasFile);
    //_executeCGALWLOP(lasFile, 1, -1, 1, false);
    //returnValue = _executeCL();  

    return returnValue;
  }
}

int main(int argc, char * argv[]) {

#ifdef DEBUG
  fmt::print("Running in DEBUG mode\n");
#endif
#ifdef _DEBUG
  fmt::print("Running in _DEBUG mode\n");
#endif
#ifdef NDEBUG
  fmt::print("Running in NDEBUG mode\n\n");
#endif

  if (argc != 2) {
    fmt::print(stderr,
               "Expected 1 parameter and got {}."
               "Please specify a valid LAS file to be loaded.\n",
               argc - 1);
    return 1;
  }

  fmt::print("Starting CLEST [{}]\n",
             boost::posix_time::to_simple_string(
               boost::posix_time::second_clock::local_time()));

  las::LASFile<-1> dummyLasFile(argv[1]);
  fmt::print("Loading LAS file:\n{}\n", argv[1]);
  dummyLasFile.loadHeaders();

  if (!dummyLasFile.isValid()) {
    fmt::print(stderr,
               "Expected a valid LAS file, but {} seems to be corrupted.\n",
               dummyLasFile.filePath);
    return 1;
  }

  int returnValue;

  if (dummyLasFile.publicHeader.pointDataRecordFormat == 0) {
    las::LASFile<0> lasFile(argv[1]);
    returnValue = _mainExecuteBlock(lasFile);
  } else if (dummyLasFile.publicHeader.pointDataRecordFormat == 1) {
    las::LASFile<1> lasFile(argv[1]);
    returnValue = _mainExecuteBlock(lasFile);
  } else if (dummyLasFile.publicHeader.pointDataRecordFormat == 2) {
    las::LASFile<2> lasFile(argv[1]);
    returnValue = _mainExecuteBlock(lasFile);
  } else if (dummyLasFile.publicHeader.pointDataRecordFormat == 3) {
    las::LASFile<3> lasFile(argv[1]);
    returnValue = _mainExecuteBlock(lasFile);
  } else {
    fmt::print(stderr,
               "Expected a valid LAS file, but the LAS uses format {},"
               "which is not valid.\n",
               dummyLasFile.publicHeader.pointDataRecordFormat);
    return 1;
  }

  fmt::print("Finished CLEST [{}]\n",
             boost::posix_time::to_simple_string(
               boost::posix_time::second_clock::local_time()));

  return returnValue;

}
