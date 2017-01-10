#include "mesher.hpp"

#include <clest/cl.hpp>

namespace {
  constexpr char BUFFER_GRID[] = "grid";
  constexpr char PROG_MARCHING[] = "marching";
  constexpr char PROG_PATH_MARCHING[] = "opencl/marching.cl";
}

namespace clest {

  void Mesher::load(grid::GridFile & grid) {
    size_t gridSize = grid.sizeX()
      * grid.sizeY()
      * grid.sizeZ()
      * sizeof(cl_uint);
    auto maxMemory = mRunner.bufferMemory();

    if (maxMemory < gridSize) {
      throw clest::Exception::build("The OpenCL context does not have enough "
                                    "memory to handle the given operation.\n"
                                    "Max allocable memory: {}B",
                                    maxMemory);
    }

    auto command = mRunner.commandQueues(1)[0];
    mRunner.createBuffer(BUFFER_GRID,
                         grid.raw()->begin(),
                         grid.raw()->end(),
                         true);
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

      auto gridSize = sizeX * sizeY * sizeZ * sizeof(cl_uint);
      auto maxMemory = mRunner.totalMemory();
      auto bufferMemory = mRunner.bufferMemory();

      if (maxMemory - sizeof(cl_uint3) <= gridSize) {
          throw clest::Exception::build("The OpenCL context does not have "
                                        "enough memory to handle the given "
                                        "operation.\n"
                                        "Max memory: {}\n"
                                        "Required:   {}",
                                        maxMemory,
                                        gridSize + sizeof(cl_uint3));
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

      auto chunkSize = std::min(maxMemory - gridSize,
                                bufferMemory) / sizeof(cl_uint3);
      size_t chunkCount = ((lasFile.pointData.size() - 1) / chunkSize) + 1;

      if (chunkCount > 1) {
        clest::println(stderr,
                       "There is not enough memory on the OpenCL context "
                       "to create the grid from a LAS file in one go.\n"
                       "It might be desirable to create the grid on the CPU.\n"
                       "Reverting to breaking the LAS in {} chunks of {}B",
                       chunkCount,
                       chunkSize * sizeof(cl_uint3));
        clest::println();
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
        mRunner.loadProgram(PROG_MARCHING,
                            PROG_PATH_MARCHING,
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

        auto gridBuffer = mRunner.createBuffer(BUFFER_GRID,
                                               CL_MEM_READ_WRITE,
                                               sizeX * sizeY * sizeZ
                                               * sizeof(cl_uint));

        auto lasBuffer = cl::Buffer(mRunner,
                                    CL_MEM_READ_ONLY,
                                    chunkSize * sizeof(cl_uint3));

        auto gridKernel = mRunner.makeKernel(PROG_MARCHING, "createGrid");
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

    clest::println();
  }
}