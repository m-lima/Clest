#include "mesher.hpp"

namespace clest {

  void Mesher::load(const grid::GridFile & grid) {
    auto command = mRunner.commandQueues(1)[0];
    auto gridBuffer = mRunner.createBuffer("grid",
                                           CL_MEM_READ_ONLY,
                                           grid.sizeX()
                                           * grid.sizeY()
                                           * grid.sizeZ());
    cl::copy(command, grid.craw()->cbegin(), grid.craw()->cend(), gridBuffer);
    command.finish();
  }

  void Mesher::load(las::LASFile<-2> & lasFile,
                    unsigned short sizeX,
                    unsigned short sizeY,
                    unsigned short sizeZ) {

      // Check validity of the parameters
      if (sizeX == 0 || sizeY == 0 || sizeZ == 0) {
        throw clest::Exception::build(
          "The size [{}, {}, {}] is invalid and must be larger than zero",
          sizeX,
          sizeY,
          sizeZ);
      }

      // Ensure all the data is laoded
      if (!lasFile.isValidAndFullyLoaded()) {
        if (!lasFile.isValid()) {
          lasFile.loadHeaders();
        }
        lasFile.loadData();

        if (!lasFile.isValidAndFullyLoaded()) {
          throw clest::Exception::build("Could not load LAS file:\n{}",
                                        lasFile.filePath);
        }
      }

      // Prepare the step sizes for creating the voxels
      float xStep = static_cast<float>(
        (lasFile.publicHeader.maxX - lasFile.publicHeader.minX)
        / (sizeX * lasFile.publicHeader.xScaleFactor));
      float xOffset = static_cast<float>(
        (lasFile.publicHeader.minX - lasFile.publicHeader.xOffset)
        / (lasFile.publicHeader.xScaleFactor));

      float yStep = static_cast<float>(
        (lasFile.publicHeader.maxY - lasFile.publicHeader.minY)
        / (sizeY * lasFile.publicHeader.yScaleFactor));
      float yOffset = static_cast<float>(
        (lasFile.publicHeader.minY - lasFile.publicHeader.yOffset)
        / (lasFile.publicHeader.yScaleFactor));

      float zStep = static_cast<float>(
        (lasFile.publicHeader.maxZ - lasFile.publicHeader.minZ)
        / (sizeZ * lasFile.publicHeader.zScaleFactor));
      float zOffset = static_cast<float>(
        (lasFile.publicHeader.minZ - lasFile.publicHeader.zOffset)
        / (lasFile.publicHeader.zScaleFactor));

      auto command = mRunner.commandQueues(1)[0];
      mRunner.loadProgram("marching", "opencl/marching.cl");

      auto gridKernel = mRunner.makeKernel
        <cl::Buffer,
        cl::Buffer,
        cl_float3,
        cl_float3,
        cl_ushort3>(
          "marching",
          "createGrid"
          );

      auto lasBuffer = mRunner.createBuffer(
        "las",
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        lasFile.pointData.size(),
        lasFile.pointData.data());

      auto gridBuffer = mRunner.createBuffer("grid",
                                             CL_MEM_READ_WRITE,
                                             sizeX * sizeY * sizeZ);

      try {
        gridKernel(cl::EnqueueArgs(command, cl::NDRange(lasFile.pointData.size())),
                   lasBuffer,
                   gridBuffer,
                   { xStep, yStep, zStep },
                   { xOffset, yOffset, zOffset },
                   { sizeX, sizeY, sizeZ });
        command.finish();
        mRunner.releaseBuffer("las");
      } catch (cl::Error & err) {
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }
  }
}