#include "mesher.hpp"

#include <clest/cl.hpp>

#include "cl/grid_program.hpp"
#include "lewiner/MarchingCubes.h"

namespace {
  constexpr char BUFFER_GRID[] = "grid";

  inline void checkMemory(size_t gridSize,
                   size_t maxMemory,
                   size_t bufferMemory) {
    if (maxMemory < gridSize) {
      throw clest::Exception::build("The OpenCL context does not have "
                                    "enough memory to handle the given "
                                    "operation.\n"
                                    "Max memory: {}\n"
                                    "Required:   {}",
                                    maxMemory,
                                    gridSize);
    }

    if (bufferMemory < gridSize) {
      clest::println(stderr,
                     "Warning: The dimensions for the grid are larger ",
                     "than the maximum allocable size.\n"
                     "This might result in instabilities\n"
                     "Max allocable: {}B\n"
                     "Required:      {}B",
                     bufferMemory,
                     gridSize);
    }
  }
}

namespace clest {

  void Mesher<GPU_DEVICE>::load(grid::GridFile && grid) {
    size_t gridSize = grid.sizeX()
      * grid.sizeY()
      * grid.sizeZ()
      * sizeof(cl_uint);
    auto maxMemory = mRunner.bufferMemory();
    auto bufferMemory = mRunner.bufferMemory();
    checkMemory(gridSize, maxMemory, bufferMemory);

    mSizeX = grid.sizeX();
    mSizeY = grid.sizeY();
    mSizeZ = grid.sizeZ();

    try {
      auto command = mRunner.commandQueues(1)[0];
      mRunner.createBuffer(BUFFER_GRID,
                           grid.raw()->begin(),
                           grid.raw()->end(),
                           true);
    } catch (cl::Error & err) {
      throw clest::Exception::build("OpenCL error: {} ({} : {})",
                                    err.what(),
                                    err.err(),
                                    ClRunner::getErrorString(err.err()));
    }
  }

  void Mesher<GPU_DEVICE>::load(las::LASFile<-2> && lasFile,
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

    auto gridSize = sizeX * sizeY * sizeZ * sizeof(cl_uint);
    auto maxMemory = mRunner.totalMemory();
    auto bufferMemory = mRunner.bufferMemory();
    checkMemory(gridSize, maxMemory - sizeof(cl_uint3), bufferMemory);

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

    auto chunkSize = std::min(maxMemory - gridSize,
                              bufferMemory) / sizeof(cl_uint3);
    size_t chunkCount = ((lasFile.pointData.size() - 1) / chunkSize) + 1;

    if (chunkCount > 1) {
      clest::println(stderr,
                     "There is not enough memory on the OpenCL context "
                     "to create the grid from a LAS file in one go.\n"
                     "Reverting to breaking the LAS in {} chunks of {}B",
                     chunkCount,
                     chunkSize * sizeof(cl_uint3));
      clest::println();
    }

    mSizeX = sizeX;
    mSizeY = sizeY;
    mSizeZ = sizeZ;

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
      mRunner.loadProgram(GridProgram(xOffset,
                                      yOffset,
                                      zOffset,
                                      xStep,
                                      yStep,
                                      zStep,
                                      sizeX,
                                      sizeY,
                                      sizeZ));

      auto gridBuffer = mRunner.createBuffer(BUFFER_GRID,
                                             CL_MEM_READ_WRITE,
                                             sizeX * sizeY * sizeZ
                                             * sizeof(cl_uint));

      auto lasBuffer = cl::Buffer(mRunner,
                                  CL_MEM_READ_ONLY,
                                  chunkSize * sizeof(cl_uint3));

      auto gridKernel = mRunner.makeKernel(GridProgram::NAME(), "createGrid");
      gridKernel.setArg(0, lasBuffer);
      gridKernel.setArg(1, gridBuffer);

      for (size_t chunk = 0; chunk < chunkCount; ++chunk) {
        clest::println("Passing LAS chunk to the device {}/{}",
                       chunk + 1,
                       chunkCount);
        size_t endPoint = std::min(((chunk + 1) * chunkSize),
                                   lasFile.pointData.size());
        cl::copy(command,
                 lasFile.pointData.begin() + (chunk * chunkSize),
                 lasFile.pointData.begin() + endPoint,
                 lasBuffer);
        auto range = endPoint - (chunk * chunkSize);

        command.enqueueNDRangeKernel(gridKernel,
                                     cl::NDRange(0),
                                     cl::NDRange(range));
        clest::println("Processing LAS chunk            {}/{}",
                       chunk + 1,
                       chunkCount);
      }
    } catch (cl::Error & err) {
      throw clest::Exception::build("OpenCL error: {} ({} : {})",
                                    err.what(),
                                    err.err(),
                                    ClRunner::getErrorString(err.err()));
    }

    mRunner.releaseProgram(GridProgram::NAME());

    clest::println();
  }
  
  void Mesher<GPU_DEVICE>::performMarchingCubes(const char * const,
                                                float) const {}

  void Mesher<CPU_DEVICE>::performMarchingCubes(const char * const path,
                                                float threshold) const {
    MarchingCubes marchingCubes;
    marchingCubes.set_resolution(mGrid.sizeX(), mGrid.sizeY(), mGrid.sizeZ());
    marchingCubes.init_all();
    clest::println("MC: [{} {} {}]",
                   marchingCubes.size_x(),
                   marchingCubes.size_y(),
                   marchingCubes.size_z());

    for (int i = 0; i < mGrid.sizeX(); ++i) {
      for (int j = 0; j < mGrid.sizeY(); ++j) {
        for (int k = 0; k < mGrid.sizeZ(); ++k) {
          marchingCubes.set_data(static_cast<real>(mGrid.cData(i, j, k)),
                                 i,
                                 j,
                                 k);
        }
      }
    }
    marchingCubes.run(mGrid.maxValue() * threshold);

    marchingCubes.clean_temps();
    marchingCubes.writePLY(path);

    marchingCubes.clean_all();
  }

}
