#pragma once
#ifndef LAS_PUBLIC_HEADER_HPP
#define LAS_PUBLIC_HEADER_HPP

#include <array>
#include <fstream>

using string = std::string;

#pragma pack(push, 1)
namespace las {
  struct PublicHeader {

    std::array<char, 4> fileSignature;
    uint16_t fileSourceID;
    uint16_t globalEncoding;
    uint32_t projectID1;
    uint16_t projectID2;
    uint16_t projectID3;
    std::array<char, 8> projectID4;
    uint8_t versionMajor;
    uint8_t versionMinor;
    std::array<char, 32> systemIdentifier;
    std::array<char, 32> generatingSoftware;
    uint16_t fileCreationDayOfYear;
    uint16_t fileCreationYear;
    uint16_t headerSize;
    uint32_t offsetToPointData;
    uint32_t numberOfVariableLengthRecords;
    uint8_t pointDataRecordFormat;
    uint16_t pointDataRecordLength;
    uint32_t legacyNumberOfPointRecords;
    std::array<uint32_t, 5> legacyNumberOfPointRecordsByReturn;
    double xScaleFactor;
    double yScaleFactor;
    double zScaleFactor;
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
    std::array<uint64_t, 15> numberOfPointsByReturn;
    
    template<class T, size_t N>
    static void insertIntoArray(std::ifstream & in, std::array<T, N> & valueArray) {
      for (auto & value : valueArray) {
        in >> value;
      }
    }

    friend std::ifstream & operator>>(std::ifstream & in, PublicHeader & header) {
      insertIntoArray(in, header.fileSignature);
      in >> header.fileSourceID;
      in >> header.globalEncoding;
      in >> header.projectID1;
      in >> header.projectID2;
      in >> header.projectID3;
      insertIntoArray(in, header.projectID4);
      in >> header.versionMajor;
      in >> header.versionMinor;
      insertIntoArray(in, header.systemIdentifier);
      insertIntoArray(in, header.generatingSoftware);
      in >> header.fileCreationDayOfYear;
      in >> header.fileCreationYear;
      in >> header.headerSize;
      in >> header.offsetToPointData;
      in >> header.numberOfVariableLengthRecords;
      in >> header.pointDataRecordFormat;
      in >> header.pointDataRecordLength;
      in >> header.legacyNumberOfPointRecords;
      insertIntoArray(in, header.legacyNumberOfPointRecordsByReturn);
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
      insertIntoArray(in, header.numberOfPointsByReturn);
      return in;
    }

  };
}
#pragma pack(pop)

#endif	// LAS_PUBLIC_HEADER_HPP

