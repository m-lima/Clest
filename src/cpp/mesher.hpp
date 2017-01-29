#pragma once

#include "las/grid_file.hpp"
#include "cl/cl_runner.hpp"

namespace clest {
  enum MesherDevice {
    CPU_DEVICE,
    GPU_DEVICE
  };

  template<MesherDevice D>
  class Mesher;

  template<>
  class Mesher<CPU_DEVICE> {
  public:
    Mesher(const grid::GridFile & grid) : mGrid(std::move(grid)) {};
    Mesher(const std::string & gridPath) : mGrid(gridPath) {};
    template <int N>
    Mesher(const las::LASFile<N> & las,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ) : mGrid(las, sizeX, sizeY, sizeZ) {};
    Mesher(const std::string lasPath,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ) :
      mGrid(las::LASFile<-2>(lasPath), sizeX, sizeY, sizeZ) {};
    
    const grid::GridFile & grid() const {
      return mGrid;
    }

    void performMarchingCubes(const char * const path, float threshold) const;

  private:
    const grid::GridFile mGrid;
  };

  template<>
  class Mesher<GPU_DEVICE> {
  public:
    // By grid
    Mesher(grid::GridFile & grid,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(grid);
    }

    Mesher(const std::string & gridPath,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(grid::GridFile(gridPath));
    }

    // By LAS
    template<int N>
    Mesher(las::LASFile<N> & las,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(las, sizeX, sizeY, sizeZ);
    }

    Mesher(const std::string & lasPath,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(las::LASFile<-2>(lasPath), sizeX, sizeY, sizeZ);
    }

    void performMarchingCubes(const char * const path, float threshold) const;

  private:
    void load(grid::GridFile & grid);
    void load(las::LASFile<-2> & lasFile,
              unsigned short sizeX,
              unsigned short sizeY,
              unsigned short sizeZ);

    unsigned short mSizeX;
    unsigned short mSizeY;
    unsigned short mSizeZ;

    clest::ClRunner mRunner;
  };
}