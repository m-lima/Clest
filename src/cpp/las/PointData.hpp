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

  template <int N>
  struct PointData;

  template<>
  struct PointData<-1> {
    uint32_t x;
    uint32_t y;
    uint32_t z;
  };

  template <>
  struct PointData<0> {
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

  template <>
  struct PointData<1> {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint16_t intensity;
    CombinedValues combinedValues;
    uint8_t classification;
    int8_t scanAngleRank;
    uint8_t userData;
    uint16_t pointSourceID;
    double GPStime;
  };

  template <>
  struct PointData<2> {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint16_t intensity;
    CombinedValues combinedValues;
    uint8_t classification;
    int8_t scanAngleRank;
    uint8_t userData;
    uint16_t pointSourceID;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
  };

  template <>
  struct PointData<3> {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint16_t intensity;
    CombinedValues combinedValues;
    uint8_t classification;
    int8_t scanAngleRank;
    uint8_t userData;
    uint16_t pointSourceID;
    double GPStime;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
  };

  template <>
  struct PointData<4> {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint16_t intensity;
    CombinedValues combinedValues;
    uint8_t classification;
    int8_t scanAngleRank;
    uint8_t userData;
    uint16_t pointSourceID;
    double GPStime;
    uint8_t wavePacketDescriptorIndex;
    uint64_t byteOffsetToWaveformLocation;
    float returnPointWaveformLocation;
    float xT;
    float yT;
    float zT;
  };

  template <>
  struct PointData<5> {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint16_t intensity;
    CombinedValues combinedValues;
    uint8_t classification;
    int8_t scanAngleRank;
    uint8_t userData;
    uint16_t pointSourceID;
    double GPStime;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint8_t wavePacketDescriptorIndex;
    uint64_t byteOffsetToWaveformLocation;
    float returnPointWaveformLocation;
    float xT;
    float yT;
    float zT;
  };
}
#pragma pack(pop)

#endif	// LAS_POINT_DATA_HPP
