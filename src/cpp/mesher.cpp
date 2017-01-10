#include "mesher.hpp"

#include <clest/cl.hpp>

namespace clest {

  void Mesher::load(const grid::GridFile & grid) {
    auto command = mRunner.commandQueues(1)[0];
    auto gridBuffer = mRunner.createBuffer("grid",
                                           CL_MEM_READ_ONLY,
                                           grid.sizeX()
                                           * grid.sizeY()
                                           * grid.sizeZ());
    cl::copy(command, grid.craw()->begin(), grid.craw()->end(), gridBuffer);
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
        clest::println("Loading las");
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
      
      try {
        auto command = mRunner.commandQueues(1)[0];
        mRunner.loadProgram("marching",
                            "opencl/marching.cl",
                            fmt::format(" -D CONST_OFFSET=(float3){{{:f},{:f},{:f}}}"
                                        " -D CONST_STEP=(float3){{{:f},{:f},{:f}}}"
                                        " -D CONST_SIZE_X={:d}"
                                        " -D CONST_SIZE_Y={:d}"
                                        " -D CONST_SIZE_Z={:d}",
                                        xOffset,
                                        yOffset,
                                        zOffset,
                                        xStep,
                                        yStep,
                                        zStep,
                                        sizeX,
                                        sizeY,
                                        sizeZ).c_str());

        auto gridKernel = mRunner.makeKernel("marching", "createGrid");
        
        clest::println("Passing LAS to the device");
        auto lasBuffer = cl::Buffer(mRunner,
                                    lasFile.pointData.begin(),
                                    lasFile.pointData.end(),
                                    true);

        auto gridBuffer = mRunner.createBuffer("grid",
                                               CL_MEM_READ_WRITE,
                                               sizeX * sizeY * sizeZ
                                               * sizeof(uint32_t));

        gridKernel.setArg(0, lasBuffer);
        gridKernel.setArg(1, gridBuffer);
        command.enqueueNDRangeKernel(gridKernel,
                                     cl::NDRange(0),
                                     cl::NDRange(lasFile.pointData.size()));
        grid::GridFile gridFile(lasFile, sizeX, sizeY, sizeZ);
        auto gridC = gridFile.craw();
        std::vector<cl_uint> gridG(sizeX * sizeY * sizeZ);

      } catch (cl::Error & err) {
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }
  }
}