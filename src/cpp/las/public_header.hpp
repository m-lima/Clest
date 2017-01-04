#pragma once

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
  };
}
#pragma pack(pop)
