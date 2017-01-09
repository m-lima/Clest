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
        clest::println("######## SIZE: {}", lasFile.pointDataCount());
        //lasFile.loadData();
        lasFile.pointData = std::vector<las::PointData<-2>>(0xA0000);
        clest::println("###### ACTUAL: {}", lasFile.pointData.size());
        clest::println("####### BYTES: {}", sizeof(las::PointData<-2>) * lasFile.pointData.size());
        clest::println("######## SENT: {}", 448 * lasFile.pointData.size());

        //if (!lasFile.isValidAndFullyLoaded()) {
        //  throw clest::Exception::build("Could not load LAS file:\n{}",
        //                                lasFile.filePath);
        //}
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

      //auto command = mRunner.commandQueues(1)[0];
      //mRunner.loadProgram("marching",
      //                    "opencl/marching.cl",
      //                    fmt::format(" -D CONST_OFFSET=(float3){{{:f},{:f},{:f}}}"
      //                                " -D CONST_STEP=(float3){{{:f},{:f},{:f}}}"
      //                                " -D CONST_SIZE_X={:d}"
      //                                " -D CONST_SIZE_Y={:d}"
      //                                " -D CONST_SIZE_Z={:d}",
      //                                xOffset,
      //                                yOffset,
      //                                zOffset,
      //                                xStep,
      //                                yStep,
      //                                zStep,
      //                                sizeX,
      //                                sizeY,
      //                                sizeZ).c_str());

      //auto gridKernel = mRunner.makeKernel
      //  <cl::Buffer,
      //  cl::Buffer>(
      //    "marching",
      //    "createGrid"
      //    );

      //auto lasBuffer = cl::Buffer(mRunner,
      //  CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
      //  lasFile.pointData.size(),
      //  lasFile.pointData.data());

      //struct GridInfo {
      //  cl_float3 step;
      //  cl_float3 offset;
      //  cl_ushort3 size;
      //};
      //std::vector<GridInfo> localInfo;
      //localInfo.emplace_back(GridInfo{ { xStep, yStep, zStep },
      //                                 { xOffset, yOffset, zOffset },
      //                                 { sizeX, sizeY, sizeZ } });

      //auto infoBuffer = cl::Buffer(mRunner, localInfo.begin(), localInfo.end(), true);

      //auto gridBuffer = cl::Buffer(mRunner,
      //                             lasFile.pointData.begin(),
      //                             lasFile.pointData.end(),
      //                             true);

      //try {
      //  gridKernel(cl::EnqueueArgs(command,
      //                             cl::NDRange(0xA0000)),
      //             lasBuffer,
      //             gridBuffer);
      //} catch (cl::Error & err) {
      //  throw clest::Exception::build("OpenCL error: {} ({})",
      //                                err.what(),
      //                                err.err());
      //}
      //  
      //command.finish();





      //auto grid = grid::GridFile(lasFile, sizeX, sizeY, sizeZ);
      //std::vector<uint32_t> gridV(sizeX * sizeY * sizeZ);
      //cl::copy(command, gridBuffer, gridV.begin(), gridV.end());

      //if (grid.craw()->size() != gridV.size()) {
      //  clest::println(stderr,
      //                 "Check does not match: [G: {} | C: {}]",
      //                 gridV.size(),
      //                 grid.craw()->size());
      //}

      //for (size_t i = 0; i < gridV.size(); ++i) {
      //  if (gridV[i] != grid.craw()->at(i)) {
      //    clest::println(stderr,
      //                   "Value mismatch at ({}): [G: {} | C: {}]",
      //                   i,
      //                   gridV[i],
      //                   grid.craw()->at(i));
      //  }
      //}
  }
}