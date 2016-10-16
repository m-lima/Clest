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

  struct PointDataZero {
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
}
#pragma pack(pop)

#endif	// LAS_POINT_DATA_HPP
