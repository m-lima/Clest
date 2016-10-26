#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <fmt/ostream.h>
#include "util.hpp"
#include "las/LASFile.hpp"
#include "colorizer.hpp"

using namespace clest;

namespace {
  const std::string _simplifyValue(double value) {
    if (value < 1000) {
      return fmt::format("{:d}", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} thousand", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} million", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} billion", value);
    }

    value /= 1000.0;
    if (value < 1000) {
      return fmt::format("{:03.2f} trillion", value);
    }

    return "<invalid>";
  }
}

int main(int argc, char ** argv) {
#ifdef DEBUG
  fmt::print("Running in DEBUG mode\n");
#endif
#ifdef _DEBUG
  fmt::print("Running in _DEBUG mode\n");
#endif
#ifdef NDEBUG
  fmt::print("Running in NDEBUG mode\n\n");
#endif
  
  if (argc != 2) {
    fmt::print(stderr, "Expected 1 parameter and got {}. Please specify a valid LAS file to be loaded.\n", argc - 1);
    return 1;
  }

  las::LASFile<las::PointData<-1>> lasFile(argv[1]);
  fmt::print("Loading LAS file:\n{}\n", argv[1]);
  //las::LASFile lasFile("C:/Users/mflim_000/Documents/VMShare/PointCloud/Liebas/Spool Dense/liebas_dense_ultra_high_snitt.las");
  lasFile.loadHeaders();

  if (!las::isLasValid(lasFile.publicHeader)) {
    fmt::print(stderr, "Expected a valid LAS file, but {} seems to be corrupted.\n", lasFile.filePath);
    return 1;
  }

#if 0
  {
    uint32_t minX = static_cast<uint32_t>((lasFile.publicHeader.minX - lasFile.publicHeader.xOffset) / lasFile.publicHeader.xScaleFactor);
    uint32_t maxX = static_cast<uint32_t>((lasFile.publicHeader.maxX - lasFile.publicHeader.xOffset) / lasFile.publicHeader.xScaleFactor);
    uint32_t minY = static_cast<uint32_t>((lasFile.publicHeader.minY - lasFile.publicHeader.yOffset) / lasFile.publicHeader.yScaleFactor);
    uint32_t maxY = static_cast<uint32_t>((lasFile.publicHeader.maxY - lasFile.publicHeader.yOffset) / lasFile.publicHeader.yScaleFactor);
    uint32_t minZ = static_cast<uint32_t>((lasFile.publicHeader.minZ - lasFile.publicHeader.zOffset) / lasFile.publicHeader.zScaleFactor);
    uint32_t maxZ = static_cast<uint32_t>((lasFile.publicHeader.maxZ - lasFile.publicHeader.zOffset) / lasFile.publicHeader.zScaleFactor);

    constexpr unsigned int FACTOR = 2;

    uint32_t deltaX = (maxX + FACTOR - minX) / FACTOR;
    uint32_t deltaY = (maxY + FACTOR - minY) / FACTOR;
    uint32_t deltaZ = (maxZ + FACTOR - minZ) / FACTOR;

    uint64_t count = 0;

    for (int i = 0; i < FACTOR; i++) {
      uint32_t x = minX + deltaX * i;
      for (int j = 0; j < FACTOR; j++) {
        uint32_t y = minY + deltaY * j;
        for (int k = 0; k < FACTOR; k++) {
          uint32_t z = minZ + deltaZ * k;
          count += lasFile.loadChunk(x, x + deltaX, y, y + deltaY, z, z + deltaZ);
          fmt::print("Total: {}\n", count);
          fmt::print(" Step: {}/{}\n\n", k + (FACTOR * j) + (FACTOR * FACTOR * i) + 1, FACTOR * FACTOR * FACTOR);
        }
      }
    }
  }
#else
  fmt::print("Loading {} points\n", _simplifyValue(static_cast<double>(lasFile.pointDataCount())));
  lasFile.loadData();
  fmt::print("Loaded {} points\n\n", _simplifyValue(static_cast<double>(lasFile.pointData.size())));
  //colorize(lasFile);
#endif

  // Verbose all platforms and devices
  if (!util::listALL()) {
    return 1;
  }

  // Choose device (and platform) that supports 64-bit
  try {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
      fmt::print(stderr, "OpenCL platforms not found.\n");
      return 1;
    }

    std::vector<cl::Device> devices;
    fmt::print("Detecting double precision devices..\n");

    fmt::print("Trying GPUs..\n");
    util::populateDevices(&platforms, &devices, CL_DEVICE_TYPE_GPU);

    if (devices.empty()) {
      fmt::print(stderr, "GPUs with double precision not found.\n");

      fmt::print("Trying other types..\n");
      util::populateDevices(&platforms, &devices, CL_DEVICE_TYPE_ALL);

      if (devices.empty()) {
        fmt::print(stderr, "No devices with double precision found.\n");
        return 1;
      }
    }

    cl::Context context(devices);

    cl::Device device = devices[0];
    fmt::print("Going for: {}\n\n", device.getInfo<CL_DEVICE_NAME>());
    cl::CommandQueue queue(context, device);

    // Establishing global range
    constexpr unsigned long long globalRepeats = 0x10000;
    constexpr unsigned int workItemCount = 0x10000;
    constexpr unsigned int workItemIterations = 0x100;
    constexpr unsigned int dataSize = workItemCount * workItemIterations;
    fmt::print("Global range: {} points\n", _simplifyValue(globalRepeats * dataSize));

    // Create data (local array and remote buffer)
    fmt::print("Creating local array");
    std::vector<unsigned char> localArray(dataSize, 0);
    fmt::print(" and remote buffer..\n");
    cl::Buffer remoteBuffer(context, CL_MEM_WRITE_ONLY, dataSize);

    // Create program
    fmt::print("Creating program..\n");
    cl::Program program(context, util::loadProgram("opencl/fractal.cl"), true);
    /* cl::make_kernel<cl::Buffer> main(program, "fractalSingle"); */
    cl::make_kernel<cl::Buffer, unsigned int> main(program, "fractalBlock");

    for (unsigned repeat = 0; repeat < 0; repeat++) {
    //for (unsigned repeat = 0; repeat < globalRepeats; repeat++) {
      // Enqueue
      /* std::cout << "Enqueueing.." << std::endl; */
      /* main(cl::EnqueueArgs(queue, cl::NDRange(workItemCount)), remoteBuffer); */
      main(cl::EnqueueArgs(queue, cl::NDRange(workItemCount)),
          remoteBuffer,
          workItemIterations);

      // Finishing
      /* std::cout << "Finishing.." << std::endl; */
      queue.finish();

      // Getting results
      /* std::cout << "Getting results.." << std::endl; */
      cl::copy(queue, remoteBuffer, localArray.begin(), localArray.end());

      /* for (size_t i = 0; i < localArray.value(); i++) { */
      /*   std::cout << static_cast<short>(localArray[i]) << ' '; */
      /* } */
    }
    fmt::print("\n");

  } catch (const cl::Error &err) {
    fmt::print(stderr, "OpenCL error: {} ({})\n", err.what(), err.err());
    return 1;
  }
}
