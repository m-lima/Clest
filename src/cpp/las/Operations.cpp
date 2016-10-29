#include "Operations.hpp"

#include <vector>

#include <fmt/format.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <CGAL/Simple_cartesian.h>
//#include <CGAL/wlop_simplify_and_regularize_point_set.h>
#include "WlopSimplifyVerbose.hpp"

#include "LASFile.hpp"
#include "PointData.hpp"

using Point3 = CGAL::Simple_cartesian<double>::Point_3;

namespace las {

  /// Convenience function to add a "tag" to a las file.
  /// Tags are identifiers such as: "wlop", "color", "new", etc.
  ///
  /// The tag will be placed right before the extension ".las"
  ///
  /// If the file does not contain a .las extension, the tag will be
  /// appended to the end of the file name and the .las extension will
  /// be added
  std::string generateName(const std::string & path, const std::string & tag) {
    // If it is empty, the name will be simply <tag>.las
    if (path.empty()) {
      return tag + ".las";
    }

    auto index = path.rfind(".las");

    // No extension
    if (index == std::string::npos) { return path + "." + tag + ".las"; }
    // Normal case
    return path.substr(0, index) + "." + tag + ".las";
  }

  /// Checks to see if the file is a valid LAS file
  /// It considers the public header sanity check and the amount of points
  /// referenced by the header (must not be empty)
  template <typename T>
  void validateLAS(const LASFile<T> & lasFile, const std::string & action) {
    if (!isLasValid(lasFile.publicHeader)) {
      throw std::runtime_error(fmt::format("Trying to {}, but {} seems to be corrupted", action, lasFile.filePath));
    }

    if (lasFile.pointDataCount() < 1) {
      throw std::runtime_error(fmt::format("Trying to {}, but {} seems to be empty", action, lasFile.filePath));
    }
  }

  /// A generic loop function to gather point data
  /// It will run in memory if the points are already loaded, except
  /// if the `forceFromFile` flag is set.
  ///
  /// If reading from memory, the function will be parallelized. The
  /// same is not true for file reading, since it would hinder performance
  ///
  /// The functor/lambda `F func` will be called for each element of the vector
  /// Also, the functor/lambda should take a `T` as parameter and a
  /// `uint64_t` current position pointer
  template <typename T, typename F>
  void mainIterator(const LASFile<T> & file, const F & func, bool forceFromFile = false) {

    // Get the point count from the header
    uint64_t dataPointCount = file.pointDataCount();

    // Decide if load from memory or from file
    if (forceFromFile || dataPointCount != file.pointData.size()) {

      // Prepare read buffer and memory pointer
      // The buffer will read as many `T` as it can pack inside BUFFER_SIZE
      /// If `sizeof(T)` exceeds `BUFFER_SIZE`, the behavior is undefined ///
      constexpr uint16_t BUFFER_SIZE = 8192;
      uint16_t typeSize = file.publicHeader.pointDataRecordLength;
      uint16_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % typeSize);

      uint64_t currentPoint = 0;
      T *base;
      char data[BUFFER_SIZE];

      // Open file
      std::ifstream fileStream(file.filePath, std::ifstream::in | std::ifstream::binary);
      if (!fileStream.is_open()) {
        throw std::runtime_error(fmt::format("Could not open file {}", file.filePath));
      }

      // Go to point data position
      fileStream.seekg(file.publicHeader.offsetToPointData);

      // While the file stream is healthy and the number of points hasn't been reached
      while (currentPoint < dataPointCount && fileStream.good()) {

        // Read the buffer
        fileStream.read(data, blockSize);

        // For each element `T`, map the memory and call `F func`
        // Add the return value to the output vector
        for (size_t i = 0; i < blockSize && currentPoint < dataPointCount; i += typeSize) {
          base = reinterpret_cast<T*>(data + i);
          func(*base, currentPoint);
          currentPoint++;
        }
      }

      // Close the stream
      fileStream.close();

      // Read from memory (in parallel)
    } else {

      // Create blocks of memory to parallelize
      tbb::blocked_range<uint64_t> block(0, file.pointData.size());

      // Wrap `F func` in a lambda running in parallel
      tbb::parallel_for(block, [&](tbb::blocked_range<uint64_t> range) {
        for (uint64_t i = range.begin(); i != range.end(); ++i) {
          func(file.pointData[i], i);
        }
      });
    }
  }

  /// Sequentially iterate over the point data and capture the min
  /// and max values for x, y, and z.
  /// Could be parallelized, but performance gain is not significant
  template <typename T>
  Limits<double> getLimits(const std::vector<T> & container) {
    Limits<double> limits;

    for (auto & point : container) {
      limits.update(point.x, point.y, point.z);
    }

    return limits;
  }

  /// Template full specialization for `Point3`
  /// since it uses a function to access the coordinates
  template<>
  Limits<double> getLimits(const std::vector<Point3> & container) {
    Limits<double> limits;

    for (auto & point : container) {
      limits.update(point.x(), point.y(), point.z());
    }

    return limits;
  }

  /// Functor to convert point data into CGAL `Point3` in a parallel fashion
  ///
  /// The `Point3` will save the coordinates in double value, i.e., it will
  /// convert the unsigned int value from LAS to double by applying the
  /// header modifiers for scale and offset
  struct PointConverter {

    // Helper variables
    const std::vector<Point3> * const _in;
    LASFile<PointData<0>> * const _out;

    const double xScale;
    const double yScale;
    const double zScale;

    const double xOffset;
    const double yOffset;
    const double zOffset;

    // Constructor to set `const` values
    PointConverter(const std::vector<Point3> & in, LASFile<PointData<0>> & out) :
      _in(&in), _out(&out),
      xScale(out.publicHeader.xScaleFactor),
      yScale(out.publicHeader.yScaleFactor),
      zScale(out.publicHeader.zScaleFactor),
      xOffset(out.publicHeader.xOffset),
      yOffset(out.publicHeader.yOffset),
      zOffset(out.publicHeader.zOffset) {}

    // Function call to convert
    void operator() (const tbb::blocked_range<uint64_t> & range) const {
      PointData<0> dummy;
      Point3 point;
      for (uint64_t i = range.begin(); i != range.end(); ++i) {
        point = (*_in)[i];
        dummy.x = static_cast<uint32_t>((point.x() - xOffset) / xScale);
        dummy.y = static_cast<uint32_t>((point.y() - yOffset) / yScale);
        dummy.z = static_cast<uint32_t>((point.z() - zOffset) / zScale);
        _out->pointData[i] = dummy;
      }
    }
  };

  /// Iterates the points from the file and changes the color according
  /// to it's position in storage
  ///
  /// The color will go from RED to BLUE from first to last data point
  ///
  /// The resulting LAS file will carry a `PointData<2>` format, the
  /// minimum necessary for a RGB point cloud
  template <typename T>
  void colorize(const LASFile<T> & lasFile) {
    validateLAS(lasFile, "colorize LAS file");

    LASFile<PointData<2>> newFile(generateName(lasFile.filePath, "color"));

    // Copy the headers and change the pertinent values
    newFile.publicHeader = lasFile.publicHeader;
    newFile.publicHeader.pointDataRecordFormat = 2;
    newFile.publicHeader.pointDataRecordLength = sizeof(PointData<2>);
    newFile.recordHeaders = lasFile.recordHeaders;

    // Prepare the color variables
    constexpr uint16_t MAX_COLOR = 0xFFFF;
    auto dataPointCount = lasFile.pointDataCount();
    newFile.pointData.resize(dataPointCount);

    mainIterator(lasFile, [&](T point, auto index) {

      // This is safe because `PointData<2>` first address that is not
      // part of the base `PointData` is the RED `uint16_t`
      // Also, `LASFile<T>` is restricted to `PointData<T>`
      PointData<2> newPoint = *reinterpret_cast<PointData<2>*>(&point);

      //// The coordinates will be the same; do an explicit memory copy
      //std::memcpy(&newPoint, &point, sizeof(uint32_t) * 3);

      // Set the colors
      newPoint.red = static_cast<uint16_t>(index * (MAX_COLOR + 1) / dataPointCount);
      newPoint.blue = static_cast<uint16_t>(MAX_COLOR - (index * (MAX_COLOR + 1) / dataPointCount));
      newPoint.green = 0;

      // Push the new value
      newFile.pointData[index] = std::move(newPoint);
    });

    newFile.save();
  }

  /// Downsamples a point cloud to `factor` percent of points
  /// without copying the whole point cloud to memory by
  /// working with reference indices instead of values
  ///
  /// The downsample picks random points from the point cloud
  /// until the desired amount of points is reached
  ///
  /// It accomplishes this by shuffling the indices of the point
  /// data and picking the first N values, where
  /// N = |pointCloud| * factor / 100
  ///
  /// `factor` should be (0 100]%
  template<typename T>
  void simplify(const LASFile<T>& lasFile, const double factor) {

    // Factor should be greater than 0 and less or equal to 100
    // factor = (0 100]
    if (!(factor > 0.0 && factor <= 100.0)) {
      throw std::runtime_error("Factor has to be from 0% to 100%");
    }
    validateLAS(lasFile, "simplify LAS");

    // Create a new file
    LASFile<T> newFile(generateName(lasFile.filePath, "simple"));

    // Calculate the new size
    uint64_t newSize = static_cast<uint64_t>(lasFile.pointDataCount() * factor / 100.0);

    // Gather the indices and shuffle them
    std::vector<uint64_t> indices;
    indices.reserve(lasFile.pointDataCount());
    for (uint64_t i = 0; i < lasFile.pointDataCount(); i++) {
      indices.push_back(i);
    }
    std::random_shuffle(indices.begin(), indices.end());

    // Pick the first N indices
    indices.resize(newSize);
    newFile.pointData.reserve(newSize);

    // If the point data is already in memory, a specific implementation
    // will be much faster than mainIterator()
    if (lasFile.pointDataCount() == lasFile.pointData.size()) {
      for (auto & index : indices) {
        newFile.pointData.push_back(lasFile.pointData[index]);
      }

      // Since it's not in memory, force it to load from file to be safe
    } else {

      // Sort them so that a stream reader can perform the capture
      // of the actual data
      std::sort(indices.begin(), indices.end());
      size_t currentIndex = 0;

      // Since the index vector is sorted, use an incrementing pointer
      // to decide wheter to add
      mainIterator(lasFile, [&](T point, auto index) {
        if (currentIndex != newSize && index == indices[currentIndex]) {
          newFile.pointData.push_back(point);
          currentIndex++;
        }
      }, true);
    }

    // Get the new limits
    Limits<double> limits = getLimits(newFile.pointData);

    // Copy the headers and change the pertinent values
    newFile.publicHeader = lasFile.publicHeader;
    newFile.publicHeader.minX = limits.minX;
    newFile.publicHeader.maxX = limits.maxX;
    newFile.publicHeader.minY = limits.minY;
    newFile.publicHeader.maxY = limits.maxY;
    newFile.publicHeader.minZ = limits.minZ;
    newFile.publicHeader.maxZ = limits.maxZ;

    newFile.recordHeaders = lasFile.recordHeaders;

    newFile.save();
  }

  /// Performs a weighted locally optimal projection of the
  /// point cloud by using CGAL's wlop
  template <typename T>
  void wlopParallel(const LASFile<T> & lasFile,
                    const double percentage,
                    const double radius,
                    const unsigned int iterations,
                    const bool uniform) {
    validateLAS(lasFile, "execute WLOP");

    // Prepare the variables for `Point3` conversion
    auto xScale = lasFile.publicHeader.xScaleFactor;
    auto yScale = lasFile.publicHeader.yScaleFactor;
    auto zScale = lasFile.publicHeader.zScaleFactor;

    auto xOffset = lasFile.publicHeader.xOffset;
    auto yOffset = lasFile.publicHeader.yOffset;
    auto zOffset = lasFile.publicHeader.zOffset;

    // Convert the points from `T` to `Point3`
    std::vector<Point3> points;
    points.reserve(lasFile.pointDataCount());
    mainIterator(lasFile, [&](T point, auto currentPoint) {
      points.push_back(Point3(point.x * xScale + xOffset,
                              point.y * yScale + yOffset,
                              point.z * zScale + zOffset));
    });
    std::vector<Point3> output;

    // Call the main wlop function
    CGAL::wlop_simplify_and_regularize_point_set<CGAL::Parallel_tag>(
      points.begin(),
      points.end(),
      std::back_inserter(output),
      percentage,
      radius,
      iterations,
      uniform
      );

    // Free the memory
    points = std::vector<Point3>();

    // Get the new limits
    Limits<double> limits = getLimits(output);

    // Copy the headers and change the pertinent values
    LASFile<PointData<0>> newFile(generateName(lasFile.filePath, "wlop"));
    newFile.publicHeader = lasFile.publicHeader;
    newFile.publicHeader.pointDataRecordFormat = 0;
    newFile.publicHeader.pointDataRecordLength = sizeof(PointData<0>);
    newFile.publicHeader.legacyNumberOfPointRecords = output.size() > 0xFFFFFFFF ? 0 : static_cast<uint32_t>(output.size());
    newFile.publicHeader.numberOfPointRecords = output.size();
    newFile.publicHeader.minX = limits.minX;
    newFile.publicHeader.maxX = limits.maxX;
    newFile.publicHeader.minY = limits.minY;
    newFile.publicHeader.maxY = limits.maxY;
    newFile.publicHeader.minZ = limits.minZ;
    newFile.publicHeader.maxZ = limits.maxZ;

    newFile.recordHeaders = lasFile.recordHeaders;

    // Convert from `Point3` back to `PointData<0>` in
    // a parallel fashion
    newFile.pointData.resize(output.size());
    tbb::blocked_range<uint64_t> block(0, output.size());
    tbb::parallel_for(block, PointConverter(output, newFile));

    // Free the memory
    output = std::vector<Point3>();

    newFile.save();
  }

  template void simplify(const LASFile<PointData<-1>> & lasFile, const double factor);
  template void colorize(const LASFile<PointData<-1>> & lasFile);
  template void wlopParallel(const LASFile<PointData<-1>> & lasFile,
                             const double percentage,
                             const double radius,
                             const unsigned int iterations,
                             const bool uniform);

  template void simplify(const LASFile<PointData<0>> & lasFile, const double factor);
  template void colorize(const LASFile<PointData<0>> & lasFile);
  template void wlopParallel(const LASFile<PointData<0>> & lasFile,
                             const double percentage,
                             const double radius,
                             const unsigned int iterations,
                             const bool uniform);

  template void simplify(const LASFile<PointData<1>> & lasFile, const double factor);
  template void colorize(const LASFile<PointData<1>> & lasFile);
  template void wlopParallel(const LASFile<PointData<1>> & lasFile,
                             const double percentage,
                             const double radius,
                             const unsigned int iterations,
                             const bool uniform);

  template void simplify(const LASFile<PointData<2>> & lasFile, const double factor);
  template void colorize(const LASFile<PointData<2>> & lasFile);
  template void wlopParallel(const LASFile<PointData<2>> & lasFile,
                             const double percentage,
                             const double radius,
                             const unsigned int iterations,
                             const bool uniform);

  template void simplify(const LASFile<PointData<3>> & lasFile, const double factor);
  template void colorize(const LASFile<PointData<3>> & lasFile);
  template void wlopParallel(const LASFile<PointData<3>> & lasFile,
                             const double percentage,
                             const double radius,
                             const unsigned int iterations,
                             const bool uniform);
}