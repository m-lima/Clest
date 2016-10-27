#ifndef CLEST_CGAL_HPP
#define CLEST_CGAL_HPP

#include <vector>
#include <fmt/ostream.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#include "las/LASFile.hpp"

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

  void convertPoints(const std::vector<Point3> & points, las::LASFile<PointData<0>> & output) {
    output.pointData.reserve(points.size());

    auto xScale = output.publicHeader.xScaleFactor;
    auto yScale = output.publicHeader.yScaleFactor;
    auto zScale = output.publicHeader.zScaleFactor;

    auto xOffset = output.publicHeader.xOffset;
    auto yOffset = output.publicHeader.yOffset;
    auto zOffset = output.publicHeader.zOffset;

    las::PointData<0> dummy;
    for (auto & point : points) {
      dummy.x = static_cast<uint32_t>((point.x() - xOffset) / xScale);
      dummy.y = static_cast<uint32_t>((point.y() - yOffset) / yScale);
      dummy.z = static_cast<uint32_t>((point.z() - zOffset) / zScale);
      output.pointData.push_back(dummy);
    }
  }
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
      2,
      0.5
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
    
    convertPoints(output, newFile);
    output = std::vector<Point3>();

    newFile.save();
  }
#endif
  void wlopSequential() {
  }
}
#endif // CLEST_CGAL_HPP