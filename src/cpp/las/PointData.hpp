#ifndef LAS_POINT_DATA_HPP
#define LAS_POINT_DATA_HPP

#pragma pack(push, 1)
namespace las {
  struct CombinedValues {
    uint8_t data;

    uint8_t getReturnNumber() {
      return data >> 5;
    }

    uint8_t getNumberOfReturns() {
      return (data >> 2) & 0b111;
    }

    bool getScanDirectionFlag() {
      return (data & 0b10) != 0;
    }

    bool getEdgeOfFlight() {
      return data & 1;
    }
  };

  struct PointDataBase {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint16_t intensity;
    CombinedValues combinedValues;
    uint8_t classification;
    int8_t scanAngleRank;
    uint8_t userData;
    uint16_t pointSourceID;
  };

  struct PointDataGPS {
    double GPStime;
  };

  struct PointDataRGB {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
  };

  struct PointDataWave {
    uint8_t wavePacketDescriptorIndex;
    uint64_t byteOffsetToWaveformLocation;
    float returnPointWaveformLocation;
    float xT;
    float yT;
    float zT;
  };

  struct PointDataZero {
    PointDataBase base;
  };

  struct PointDataOne {
    PointDataBase base;
    PointDataGPS gps;
  };

  struct PointDataTwo {
    PointDataBase base;
    PointDataRGB rgb;
  };

  struct PointDataThree {
    PointDataBase base;
    PointDataGPS gps;
    PointDataRGB rgb;
  };

  struct PointDataFour {
    PointDataBase base;
    PointDataGPS gps;
    PointDataWave wave;
  };

  struct PointDataFive {
    PointDataBase base;
    PointDataGPS gps;
    PointDataRGB rgb;
    PointDataWave wave;
  };

  struct PointDataMin {
    uint32_t x;
    uint32_t y;
    uint32_t z;

    PointDataMin(const PointDataBase & pointData) :
      x(pointData.x),
      y(pointData.y),
      z(pointData.z) {}
  };
}
#pragma pack(pop)

#endif	// LAS_POINT_DATA_HPP
