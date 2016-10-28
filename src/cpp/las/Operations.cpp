#include "Operations.hpp"

#include <vector>

#include <fmt/ostream.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <CGAL/Simple_cartesian.h>
//#include <CGAL/wlop_simplify_and_regularize_point_set.h>
#include "WlopSimplifyVerbose.hpp"

#include "LASFile.hpp"
#include "PointData.hpp"

/***********
* FUNCTOR FILE ITERATOR
*
* CHECK IF DATA IS ALREADY LOADED
*
*
*
*
*
*
*
*
* T
* O
* |
* D
* O
*
*
*
*
**************/

using Point3 = CGAL::Simple_cartesian<double>::Point_3;

namespace las {

  std::string generateName(const std::string & path, const std::string & tag) {
    if (path.empty()) {
      return tag + ".las";
    }

    auto index = path.rfind(".las");
    if (index == std::string::npos) { return path + "." + tag + ".las"; }
    return path.substr(0, index) + "." + tag + ".las";
  }

  template <typename T>
  void validateLAS(const LASFile<T> & lasFile, const std::string & action) {
    if (!isLasValid(lasFile.publicHeader)) {
      throw std::runtime_error(fmt::format("Trying to {}, but {} seems to be corrupted", action, lasFile.filePath));
    }

    if (lasFile.pointDataCount() < 1) {
      throw std::runtime_error(fmt::format("Trying to {}, but {} seems to be empty", action, lasFile.filePath));
    }
  }

  template <typename T>
  std::vector<T> loadRandomPoints(const LASFile<T> & file, const vector<uint64_t> & indices) {

  }

  template <typename T>
  std::vector<PointData<2>> iterateAndSetColor(const LASFile<T> & file) {
    constexpr uint16_t BUFFER_SIZE = 8192;
    constexpr uint16_t MAX_COLOR = 0xFFFF;

    uint64_t dataPointCount = file.pointDataCount();
    uint16_t typeSize = file.publicHeader.pointDataRecordLength;

    uint64_t currentPoint = 0;
    uint16_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % typeSize);

    PointData<2> newPoint;
    newPoint.green = 0;
    std::vector<PointData<2>> output;
    output.reserve(dataPointCount);
    T *base;
    char data[BUFFER_SIZE];

    std::ifstream fileStream(file.filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", file.filePath));
    }

    fileStream.seekg(file.publicHeader.offsetToPointData);

    while (currentPoint < dataPointCount && fileStream.good()) {
      fileStream.read(data, blockSize);
      for (size_t i = 0; i < blockSize && currentPoint < dataPointCount; i += typeSize) {
        base = reinterpret_cast<T*>(data + i);

        newPoint.x = base->x;
        newPoint.y = base->y;
        newPoint.z = base->z;
        newPoint.red = static_cast<uint16_t>(currentPoint * (MAX_COLOR + 1) / dataPointCount);
        newPoint.blue = static_cast<uint16_t>(MAX_COLOR - (currentPoint * (MAX_COLOR + 1) / dataPointCount));
        output.push_back(newPoint);
        currentPoint++;
      }
    }

    fileStream.close();
    return output;
  }

  template <typename T>
  std::vector<Point3> loadIntoGCALPoints(const LASFile<T> & file) {
    constexpr uint16_t BUFFER_SIZE = 8192;

    uint64_t dataPointCount = file.pointDataCount();
    uint16_t typeSize = file.publicHeader.pointDataRecordLength;

    uint64_t currentPoint = 0;
    uint16_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % typeSize);

    auto xScale = file.publicHeader.xScaleFactor;
    auto yScale = file.publicHeader.yScaleFactor;
    auto zScale = file.publicHeader.zScaleFactor;

    auto xOffset = file.publicHeader.xOffset;
    auto yOffset = file.publicHeader.yOffset;
    auto zOffset = file.publicHeader.zOffset;

    std::vector<Point3> output;
    output.reserve(dataPointCount);
    T *base;
    char data[BUFFER_SIZE];

    std::ifstream fileStream(file.filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file {}", file.filePath));
    }

    fileStream.seekg(file.publicHeader.offsetToPointData);

    while (currentPoint < dataPointCount && fileStream.good()) {
      fileStream.read(data, blockSize);
      for (size_t i = 0; i < blockSize && currentPoint < dataPointCount; i += typeSize) {
        base = reinterpret_cast<T*>(data + i);
        output.push_back(Point3(base->x * xScale + xOffset,
                                base->y * yScale + yOffset,
                                base->z * zScale + zOffset));
        currentPoint++;
      }
    }

    fileStream.close();
    return output;
  }

  Limits<double> getLimits(const std::vector<Point3> & container) {
    Limits<double> limits;

    for (auto & point : container) {
      limits.update(point.x(), point.y(), point.z());
    }

    return limits;
  }

  struct PointConverter {
    const std::vector<Point3> * const _in;
    LASFile<PointData<0>> * const _out;

    const double xScale;
    const double yScale;
    const double zScale;

    const double xOffset;
    const double yOffset;
    const double zOffset;

    PointConverter(const std::vector<Point3> & in, LASFile<PointData<0>> & out) :
      _in(&in), _out(&out),
      xScale(out.publicHeader.xScaleFactor),
      yScale(out.publicHeader.yScaleFactor),
      zScale(out.publicHeader.zScaleFactor),
      xOffset(out.publicHeader.xOffset),
      yOffset(out.publicHeader.yOffset),
      zOffset(out.publicHeader.zOffset) {}

    void operator() (const tbb::blocked_range<uint32_t> & range) const {
      PointData<0> dummy;
      Point3 point;
      for (uint32_t i = range.begin(); i != range.end(); ++i) {
        point = (*_in)[i];
        dummy.x = static_cast<uint32_t>((point.x() - xOffset) / xScale);
        dummy.y = static_cast<uint32_t>((point.y() - yOffset) / yScale);
        dummy.z = static_cast<uint32_t>((point.z() - zOffset) / zScale);
        _out->pointData[i] = dummy;
      }
    }
  };

  template <typename T>
  void colorize(const LASFile<T> & lasFile) {
    validateLAS(lasFile, "colorize LAS file");

    LASFile<PointData<2>> newFile(generateName(lasFile.filePath, "color"));

    newFile.publicHeader = lasFile.publicHeader;
    newFile.publicHeader.pointDataRecordFormat = 2;
    newFile.publicHeader.pointDataRecordLength = sizeof(PointData<2>);
    newFile.recordHeaders = lasFile.recordHeaders;

    iterateAndSetColor(lasFile);

    newFile.save();
  }

  template<typename T>
  void simplify(const LASFile<T>& lasFile, const double factor) {
    if (!(factor > 0.0 && factor <= 100.0)) {
      throw std::runtime_error("Factor has to be from 0% to 100%");
    }
    validateLAS(lasFile, "simplify LAS");

    LASFile<T> newFile(generateName(lasFile.filePath, "simple"));

    uint64_t newSize = lasFile.pointDataCount() * factor / 100.0;

    newFile.publicHeader = lasFile.publicHeader;
    newFile.recordHeaders = lasFile.recordHeaders;

    std::vector<uint64_t> indices;
    indices.reserve(lasFile.pointDataCount());
    for (uint64_t i = 0; i < lasFile.pointDataCount(); i++) {
      indices.push_back(i);
    }
    std::random_shuffle(indices.begin(), indices.end());

    indices.resize(newSize);
    std::sort(indices.begin(), indices.end());

    newFile.pointData = loadRandomPoints(lasFile, indices);
  }

#ifdef CGAL_LINKED_WITH_TBB
  template <typename T>
  void wlopParallel(const LASFile<T> & lasFile) {
    validateLAS(lasFile, "execute WLOP");
    
    std::vector<Point3> points = loadIntoGCALPoints(lasFile);
    std::vector<Point3> output;

    double percentage = 0.1;
    double radius = 0.1;
    unsigned int iterations = 10;
    bool uniform = false;
    fmt::print("Parameters:\nNumber of points: {}\nPercentage to keep: {}%\nRadius: {}\nNumber of iterations: {}\nRequires uniform: {}\n\n", points.size(), percentage, radius, iterations, uniform);
    CGAL::wlop_simplify_and_regularize_point_set<CGAL::Parallel_tag>(
      points.begin(),
      points.end(),
      std::back_inserter(output),
      percentage,
      radius,
      iterations,
      uniform
      );

    points = std::vector<Point3>();
    Limits<double> limits = getLimits(output);

    LASFile<PointData<0>> newFile(generateName(lasFile.filePath, "wlop"));
    newFile.publicHeader = lasFile.publicHeader;
    newFile.publicHeader.pointDataRecordFormat = 0;
    newFile.publicHeader.pointDataRecordLength = sizeof(PointData<0>);
    newFile.publicHeader.legacyNumberOfPointRecords = static_cast<uint32_t>(output.size());
    newFile.publicHeader.numberOfPointRecords = output.size();
    newFile.publicHeader.minX = limits.minX;
    newFile.publicHeader.maxX = limits.maxX;
    newFile.publicHeader.minY = limits.minY;
    newFile.publicHeader.maxY = limits.maxY;
    newFile.publicHeader.minZ = limits.minZ;
    newFile.publicHeader.maxZ = limits.maxZ;

    newFile.recordHeaders = lasFile.recordHeaders;

    newFile.pointData.resize(output.size());
    tbb::blocked_range<uint32_t> block(0, output.size());
    tbb::parallel_for(block, PointConverter(output, newFile));

    output = std::vector<Point3>();

    newFile.save();
  }

  template void wlopParallel(const LASFile<PointData<-1>> & lasFile);
#endif
}