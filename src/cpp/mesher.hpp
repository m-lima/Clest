#pragma once

#include "las/grid_file.hpp"
#include "las/public_header.hpp"
#include "cl/cl_runner.hpp"

namespace clest {
  class Mesher {
  public:
    Mesher(const grid::GridFile & grid,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(grid);
    }

    Mesher(las::LASFile<-2> & las,
           unsigned short sizeX,
           unsigned short sizeY,
           unsigned short sizeZ,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(las, sizeX, sizeY, sizeZ);
    }

    Mesher(const std::string & gridPath,
           cl_device_type type = CL_DEVICE_TYPE_GPU,
           const std::vector<const char*> & requirements = { "fp64" }) :
      mRunner(type, requirements) {
      load(grid::GridFile(gridPath));
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

  private:
    void load(const grid::GridFile & grid);
    void load(las::LASFile<-2> & lasFile,
              unsigned short sizeX,
              unsigned short sizeY,
              unsigned short sizeZ);

    clest::ClRunner mRunner;
  };
}