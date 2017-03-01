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
    Mesher(const std::string & lasPath,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ,
           bool ignoreColor = false) {
      if (ignoreColor) {
        mGrid = grid::GridFile(las::LASFile<-2>(lasPath), sizeX, sizeY, sizeZ);
      } else {
        las::LASFile<-2> las(lasPath);
        las.loadHeaders();
        auto format = las.publicHeader.pointDataRecordFormat;
        if (format == 2 || format == 3 || format == 5) {
          mGrid = grid::GridFile(las::LASFile<-3>(lasPath), sizeX, sizeY, sizeZ);
        } else {
          mGrid = grid::GridFile(std::move(las), sizeX, sizeY, sizeZ);
        }
      }
    }

    const grid::GridFile & grid() const {
      return mGrid;
    }

    void performMarchingCubes(const char * const path, float threshold) const;

  private:
    grid::GridFile mGrid;
  };

  template<>
  class Mesher<GPU_DEVICE> {
  public:
    // By grid
    Mesher(grid::GridFile & grid,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(std::move(grid));
    }

    Mesher(const std::string & gridPath,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(grid::GridFile(gridPath));
    }

    // By LAS
    Mesher(las::LASFile<-2> & las,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements),
      mSizeX(sizeX),
      mSizeY(sizeY),
      mSizeZ(sizeZ) {
      load(std::move(las));
    }

    Mesher(las::LASFile<-3> & las,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements),
      mSizeX(sizeX),
      mSizeY(sizeY),
      mSizeZ(sizeZ) {
      load(std::move(las));
    }

    Mesher(const std::string & lasPath,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ,
           bool ignoreColor = false,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements),
      mSizeX(sizeX),
      mSizeY(sizeY),
      mSizeZ(sizeZ) {
      if (ignoreColor) {
        load(las::LASFile<-2>(lasPath));
      } else {
        las::LASFile<-2> las(lasPath);
        las.loadHeaders();
        auto format = las.publicHeader.pointDataRecordFormat;
        if (format == 2 || format == 3 || format == 5) {
          load(las::LASFile<-3>(lasPath));
        } else {
          load(std::move(las));
        }
      }
    }

    void performMarchingCubes(const char * const path, float threshold) const;

  private:
    void load(grid::GridFile && grid);

    template<int N>
    void load(las::LASFile<N> && lasFile);

    unsigned short mSizeX;
    unsigned short mSizeY;
    unsigned short mSizeZ;

    clest::ClRunner mRunner;
  };
}
