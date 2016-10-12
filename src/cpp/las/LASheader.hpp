#ifndef LASHEADER_H
#define LASHEADER_H

#include <string>
#include <iostream>
#include <fstream>

using string = std::string;

namespace las {
  struct LASheader {
    template<uint8_t MAX>
    struct LimitedValues {
      uint64_t & operator [](int i) {
        if (i < MAX) {
          return value[i];
        }
        throw std::out_of_range("Number of points by return access at: " + std::to_string(i) + " where max is: " + std::to_string(MAX));
      }
      
      friend std::ifstream & operator>>(std::ifstream & in, LimitedValues & limitedValue) {
        for (int i = 0; i < MAX; i++) {
          in >> limitedValue.value[i];
        }
        return in;
      }

    private:
      uint64_t value[MAX];
    };

    using LegacyNumberOfPointRecordsByReturn = LimitedValues<5>;
    using NumberOfPointsByReturn = LimitedValues<15>;

    static constexpr uint16_t MAX_BYTE_SIZE = 375;
    static constexpr uint8_t LEGACY_NUMBER_OF_POINT_RECORDS_BY_RETURN_SIZE = 5;
    static constexpr uint8_t NUMBER_OF_POINTS_BY_RETURN_SIZE = 15;

    string fileSignature = "LASF";
    uint16_t fileSourceID;
    uint16_t globalEncoding;
    uint32_t projectID1;
    uint16_t projectID2;
    uint16_t projectID3;
    string projectID4 = "";
    uint8_t versionMajor = 1;
    uint8_t versionMinor = 2;
    string systemIdentifier = "MFLima";
    string generatingSoftware = "MFLima PointCLoud";
    uint16_t fileCreationDayOfYear;
    uint16_t fileCreationYear;
    uint16_t headerSize = 227;
    uint32_t offsetToPointData = 227;
    uint32_t numberOfVariableLengthRecords;
    uint8_t pointDataRecordFormat;
    uint16_t pointDataRecordLength = 20;
    uint32_t legacyNumberOfPointRecords;
    LegacyNumberOfPointRecordsByReturn legacyNumberOfPointRecordsByReturn;
    double xScaleFactor = 1;
    double yScaleFactor = 1;
    double zScaleFactor = 1;
    double xOffset;
    double yOffset;
    double zOffset;
    double maxX;
    double minX;
    double maxY;
    double minY;
    double maxZ;
    double minZ;
    uint64_t startOfWaveformDataPacketRecord;
    uint64_t startOfFirstExtendedVariableLengthRecord;
    uint32_t numberOfExtendedVariableLengthRecords;
    uint64_t numberOfPointRecords;
    NumberOfPointsByReturn numberOfPointsByReturn;

    friend std::ifstream & operator>>(std::ifstream & in, LASheader & header) {
      std::cout << header.fileSignature << std::endl;
      std::cout << header.versionMajor << '.' << header.versionMinor << std::endl;
      in >> header.fileSignature;
      in >> header.fileSourceID;
      in >> header.globalEncoding;
      in >> header.projectID1;
      in >> header.projectID2;
      in >> header.projectID3;
      in >> header.projectID4;
      in >> header.versionMajor;
      in >> header.versionMinor;
      in >> header.systemIdentifier;
      in >> header.generatingSoftware;
      in >> header.fileCreationDayOfYear;
      in >> header.fileCreationYear;
      in >> header.headerSize;
      in >> header.offsetToPointData;
      in >> header.numberOfVariableLengthRecords;
      in >> header.pointDataRecordFormat;
      in >> header.pointDataRecordLength;
      in >> header.legacyNumberOfPointRecords;
      in >> header.legacyNumberOfPointRecordsByReturn;
      in >> header.xScaleFactor;
      in >> header.yScaleFactor;
      in >> header.zScaleFactor;
      in >> header.xOffset;
      in >> header.yOffset;
      in >> header.zOffset;
      in >> header.maxX;
      in >> header.minX;
      in >> header.maxY;
      in >> header.minY;
      in >> header.maxZ;
      in >> header.minZ;
      in >> header.startOfWaveformDataPacketRecord;
      in >> header.startOfFirstExtendedVariableLengthRecord;
      in >> header.numberOfExtendedVariableLengthRecords;
      in >> header.numberOfPointRecords;
      in >> header.numberOfPointsByReturn;
      std::cout << header.fileSignature << std::endl;
      std::cout << header.versionMajor << '.' << header.versionMinor << std::endl;
      return in;
    }

  };
}

#endif	// LASHEADER_H

