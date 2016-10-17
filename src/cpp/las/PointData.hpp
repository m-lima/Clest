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

    //PointDataBase() = default; // Constructor
    //virtual ~PointDataBase() = default; //Destructor
    //PointDataBase(const PointDataBase &) = default; // Copy constructor
    //PointDataBase & operator=(const PointDataBase &) = default; // Copy assignement
    //PointDataBase(PointDataBase &&) = default; // Move constructor
    //PointDataBase & operator=(PointDataBase &&) = default; // Move assignment
  };

  struct PointDataGPS {
    double GPStime;
    
    //PointDataGPS() = default; // Constructor
    //virtual ~PointDataGPS() = default; //Destructor
    //PointDataGPS(const PointDataGPS &) = default; // Copy constructor
    //PointDataGPS & operator=(const PointDataGPS &) = default; // Copy assignement
    //PointDataGPS(PointDataGPS &&) = default; // Move constructor
    //PointDataGPS & operator=(PointDataGPS &&) = default; // Move assignment
  };

  struct PointDataRGB {
    uint16_t red;
    uint16_t green;
    uint16_t blue;

  //  PointDataRGB() = default; // Constructor
  //  virtual ~PointDataRGB() = default; //Destructor
  //  PointDataRGB(const PointDataRGB &) = default; // Copy constructor
  //  PointDataRGB & operator=(const PointDataRGB &) = default; // Copy assignement
  //  PointDataRGB(PointDataRGB &&) = default; // Move constructor
  //  PointDataRGB & operator=(PointDataRGB &&) = default; // Move assignment
  };

  struct PointDataWave {
    uint8_t wavePacketDescriptorIndex;
    uint64_t byteOffsetToWaveformLocation;
    float returnPointWaveformLocation;
    float xT;
    float yT;
    float zT;

  //  PointDataWave() = default; // Constructor
  //  virtual ~PointDataWave() = default; //Destructor
  //  PointDataWave(const PointDataWave &) = default; // Copy constructor
  //  PointDataWave & operator=(const PointDataWave &) = default; // Copy assignement
  //  PointDataWave(PointDataWave &&) = default; // Move constructor
  //  PointDataWave & operator=(PointDataWave &&) = default; // Move assignment
  };

  //template<int N>
  //struct PointData {};

  //template<>
  //struct PointData<0> : PointDataBase {};

  //template<>
  //struct PointData<1> : PointDataBase, PointDataGPS {};

  //template<>
  //struct PointData<2> : PointDataBase, PointDataRGB {};

  //template<>
  //struct PointData<3> : PointData<1>, PointDataRGB {};

  //template<>
  //struct PointData<4> : PointData<1>, PointDataWave {};

  //template<>
  //struct PointData<5> : PointData<3>, PointDataWave {};

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
