#ifndef CLEST_CGAL_HPP
#define CLEST_CGAL_HPP

#include <vector>

#include <fmt/ostream.h>

#include <CGAL/Simple_cartesian.h>
//#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include "las/LASFile.hpp"
#include "wlopSimplifyVerbose.hpp"

using Point3 = CGAL::Simple_cartesian<double>::Point_3;

namespace {

  template <typename T>
  std::vector<Point3> loadIntoGCALPoints(const las::LASFile<T> & file) {
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
    las::LASFile<PointData<0>> * const _out;

    const double xScale;
    const double yScale;
    const double zScale;

    const double xOffset;
    const double yOffset;
    const double zOffset;

    PointConverter(const std::vector<Point3> & in, las::LASFile<PointData<0>> & out) :
      _in(&in), _out(&out),
      xScale(out.publicHeader.xScaleFactor),
      yScale(out.publicHeader.yScaleFactor),
      zScale(out.publicHeader.zScaleFactor),
      xOffset(out.publicHeader.xOffset),
      yOffset(out.publicHeader.yOffset),
      zOffset(out.publicHeader.zOffset) {}

    void operator() (const tbb::blocked_range<uint32_t> & range) const {
      las::PointData<0> dummy;
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
}

namespace clest {
#ifdef CGAL_LINKED_WITH_TBB
  template <typename T>
  void wlopParallel(const las::LASFile<T> & lasFile) {
    if (!isLasValid(lasFile.publicHeader)) {
      fmt::print(stderr, "Trying to colorize LAS file, but {} seems to be corrupted.\n", lasFile.filePath);
    }

    if (lasFile.pointDataCount() < 1) {
      fmt::print(stderr, "Trying to colorize LAS file, but {} seems to be empty.\n", lasFile.filePath);
    }

    std::vector<Point3> points = loadIntoGCALPoints(lasFile);
    std::vector<Point3> output;

    CGAL::wlop_simplify_and_regularize_point_set<CGAL::Parallel_tag>(
      points.begin(),
      points.end(),
      std::back_inserter(output),
      10,
      1,
      35
      );

    points = std::vector<Point3>();
    Limits<double> limits = getLimits(output);

    las::LASFile<las::PointData<0>> newFile(lasFile.filePath + ".wlop.las");
    newFile.publicHeader = lasFile.publicHeader;
    newFile.publicHeader.pointDataRecordFormat = 0;
    newFile.publicHeader.pointDataRecordLength = sizeof(las::PointData<0>);
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
#endif
  void wlopSequential() {
  }
}
#endif // CLEST_CGAL_HPP