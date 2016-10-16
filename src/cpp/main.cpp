#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include "util.hpp"
#include "las/PublicHeader.hpp"
#include "las/FileHandler.hpp"

using namespace clest;

int main(int argc, char ** argv) {

  // Verbose all platforms and devices
  if (!util::listALL()) {
    return 1;
  }

  if (argc != 2) {
    std::cerr << "Wrong parameters" << std::endl;
    return 1;
  }

  las::LASFile lasFile = las::read(argv[1]);
  //las::LASFile lasFile = las::read("C:/Users/mflim_000/Documents/VMShare/PointCloud/Liebas/Full Boat/liebas_dense_high_snitt.las");
  //las::LASFile lasFile = las::read("C:/Users/mflim_000/Documents/VMShare/PointCloud/Liebas/Spool Dense/liebas_dense_ultra_high_snitt.las");
  //las::LASFile lasFile = las::read("C:/Users/mflim_000/Documents/VMShare/PointCloud/Liebas/Spool Dense/liebas_dense_ultra_high_snitt.las.las");
  //las::LASFile lasFile = las::read("C:/Users/mflim_000/Documents/VMShare/PointCloud/Liebas/Spool Dense/liebas_dense_ultra_high_snitt2.las");
  //las::LASFile lasFile = las::read("C:/Users/mflim_000/Documents/VMShare/PointCloud/Liebas/Spool Dense/SinglePoint.las");

  // Choose device (and platform) that supports 64-bit
  //try {
  //  std::vector<cl::Platform> platforms;
  //  cl::Platform::get(&platforms);

  //  if (platforms.empty()) {
  //    std::cerr << "OpenCL platforms not found." << std::endl;
  //    return 1;
  //  }

  //  std::vector<cl::Device> devices;
  //  std::cout << "Detecting double precision devices.." << std::endl;

  //  std::cout << "Trying GPUs.." << std::endl;
  //  util::populateDevices(&platforms, &devices, CL_DEVICE_TYPE_GPU);

  //  if (devices.empty()) {
  //    std::cerr << "GPUs with double precision not found." << std::endl;

  //    std::cout << "Trying other types.." << std::endl;
  //    util::populateDevices(&platforms, &devices, CL_DEVICE_TYPE_ALL);

  //    if (devices.empty()) {
  //      std::cerr << "No devices with double precision found." << std::endl;
  //      return 1;
  //    }
  //  }

  //  cl::Context context(devices);

  //  cl::Device device = devices[0];
  //  std::cout << "Going for: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
  //  cl::CommandQueue queue(context, device);

  //  // Establishing global range
  //  constexpr unsigned long long globalRepeats = 0x10000;
  //  constexpr unsigned int workItemCount = 0x10000;
  //  constexpr unsigned int workItemIterations = 0x100;
  //  constexpr unsigned int dataSize = workItemCount * workItemIterations;
  //  std::cout << "Global range: "
  //    << std::setprecision(2) << std::fixed
  //    << (globalRepeats * dataSize) / 1000000000.0
  //    << " billion points" << std::endl;

  //  // Create data (local array and remote buffer)
  //  std::cout << "Creating local array";
  //  std::vector<unsigned char> localArray(dataSize, 0);
  //  std::cout << " and remote buffer.." << std::endl;
  //  cl::Buffer remoteBuffer(context, CL_MEM_WRITE_ONLY, dataSize);

  //  // Create program
  //  std::cout << "Creating program.." << std::endl;
  //  cl::Program program(context, util::loadProgram("opencl/fractal.cl"), true);
  //  /* cl::make_kernel<cl::Buffer> main(program, "fractalSingle"); */
  //  cl::make_kernel<cl::Buffer, unsigned int> main(program, "fractalBlock");

  //  for (unsigned repeat = 0; repeat < globalRepeats; repeat++) {
  //    // Enqueue
  //    /* std::cout << "Enqueueing.." << std::endl; */
  //    /* main(cl::EnqueueArgs(queue, cl::NDRange(workItemCount)), remoteBuffer); */
  //    main(cl::EnqueueArgs(queue, cl::NDRange(workItemCount)),
  //        remoteBuffer,
  //        workItemIterations);

  //    // Finishing
  //    /* std::cout << "Finishing.." << std::endl; */
  //    queue.finish();

  //    // Getting results
  //    /* std::cout << "Getting results.." << std::endl; */
  //    cl::copy(queue, remoteBuffer, localArray.begin(), localArray.end());

  //    /* for (size_t i = 0; i < localArray.size(); i++) { */
  //    /*   std::cout << static_cast<short>(localArray[i]) << ' '; */
  //    /* } */
  //  }

  //  std::cout << std::endl;

  //} catch (const cl::Error &err) {
  //  std::cerr
  //    << "OpenCL error: "
  //    << err.what() << '(' << err.err() << ')'
  //    << std::endl;
  //  return 1;
  //}
}
