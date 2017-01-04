#pragma once

#include <limits>

namespace las {
  template <typename T>
  struct Limits {
    T minX = std::numeric_limits<T>::max();
    T maxX = std::numeric_limits<T>::min();
    T minY = std::numeric_limits<T>::max();
    T maxY = std::numeric_limits<T>::min();
    T minZ = std::numeric_limits<T>::max();
    T maxZ = std::numeric_limits<T>::min();

    Limits() = default;
    Limits(T _minX, T _maxX, T _minY, T _maxY, T _minZ, T _maxZ) :
      minX(_minX),
      maxX(_maxX),
      minY(_minY),
      maxY(_maxY),
      minZ(_minZ),
      maxZ(_maxZ) {}

    bool isMaxed() const {
      return
        minX == std::numeric_limits<T>::max() &&
        maxX == std::numeric_limits<T>::min() &&
        minY == std::numeric_limits<T>::max() &&
        maxY == std::numeric_limits<T>::min() &&
        minZ == std::numeric_limits<T>::max() &&
        maxZ == std::numeric_limits<T>::min();
    }

    bool isOutside(T x, T y, T z) const {
      return x < minX || y < minY || z < minZ
        || x >= maxX || y >= maxY || z >= maxZ;
    }

    void update(T x, T y, T z) {
      if (x < minX) { minX = x; }
      if (x > maxX) { maxX = x; }
      if (y < minY) { minY = y; }
      if (y > maxY) { maxY = y; }
      if (z < minZ) { minZ = z; }
      if (z > maxZ) { maxZ = z; }
    }
  };

#define _PART_XYZ uint32_t x;\
                  uint32_t y;\
                  uint32_t z

#define _PART_BASIC uint16_t intensity;\
                    CombinedValues combinedValues;\
                    uint8_t classification;\
                    int8_t scanAngleRank;\
                    uint8_t userData;\
                    uint16_t pointSourceID

#define _PART_RGB uint16_t red;\
                  uint16_t green;\
                  uint16_t blue

#define _PART_WAVE uint8_t wavePacketDescriptorIndex;\
                   uint64_t byteOffsetToWaveformLocation;\
                   float returnPointWaveformLocation;\
                   float xT;\
                   float yT;\
                   float zT

#pragma pack(push, 1)
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
    static constexpr int FORMAT = -1;
    _PART_XYZ;
  };

  template <>
  struct PointData<0> {
    static constexpr int FORMAT = 0;
    _PART_XYZ;
    _PART_BASIC;
  };

  template <>
  struct PointData<1> {
    static constexpr int FORMAT = 1;
    _PART_XYZ;
    _PART_BASIC;
    double GPStime;
  };

  template <>
  struct PointData<2> {
    static constexpr int FORMAT = 2;
    _PART_XYZ;
    _PART_BASIC;
    _PART_RGB;
  };

  template <>
  struct PointData<3> {
    static constexpr int FORMAT = 3;
    _PART_XYZ;
    _PART_BASIC;
    double GPStime;
    _PART_RGB;
  };

  template <>
  struct PointData<4> {
    static constexpr int FORMAT = 4;
    _PART_XYZ;
    _PART_BASIC;
    double GPStime;
    _PART_WAVE;
  };

  template <>
  struct PointData<5> {
    static constexpr int FORMAT = 5;
    _PART_XYZ;
    _PART_BASIC;
    double GPStime;
    _PART_RGB;
    _PART_WAVE;
  };
#pragma pack(pop)

#undef _PART_XYZ
#undef _PART_BASIC
#undef _PART_RGB
#undef _PART_WAVE
}
