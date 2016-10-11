#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <LASlib/lasreader.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include "util.hpp"

using namespace clest;

int main() {

  // Verbose all platforms and devices
  if (!util::listALL()) {
    return 1;
  }

  LASreadOpener readOpener;
  readOpener.set_file_name("Yo");
  if (!readOpener.active()) {
    std::cerr << "File not found." << std::endl;
    return 1;
  }

  std::cout << std::endl;

  // Choose device (and platform) that supports 64-bit
  try {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
      std::cerr << "OpenCL platforms not found." << std::endl;
      return 1;
    }

    cl::Context context;
    std::vector<cl::Device> devices;
    for (auto platform = platforms.begin(); devices.empty() && platform != platforms.end(); platform++) {
      std::vector<cl::Device> platformDevices;

      try {
        platform->getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);

        for (auto device = platformDevices.begin(); devices.empty() && device != platformDevices.end(); device++) {
          std::string ext = device->getInfo<CL_DEVICE_EXTENSIONS>();

          if (ext.find("cl_khr_fp64") == std::string::npos) {
            if (ext.find("cl_amd_fp64") == std::string::npos) {
              continue;
            }
          }

          devices.push_back(*device);
          context = cl::Context(devices);
        }

      } catch (...) {
        devices.clear();
      }
    }

    if (devices.empty()) {
      std::cerr << "GPUs with double precision not found." << std::endl;
      return 1;
    }

    cl::Device device = devices[0];
    std::cout << "Going for: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    cl::CommandQueue queue(context, device);

    // Establishing global range
    constexpr unsigned long long globalRepeats = 0x10000;
    constexpr unsigned int workItemCount = 0x10000;
    constexpr unsigned int workItemIterations = 0x100;
    constexpr unsigned int dataSize = workItemCount * workItemIterations;
    std::cout << "Global range: " << std::setprecision(2) << std::fixed << (globalRepeats * dataSize) / 1000000000.0 << " billion points" << std::endl;

    // Create data (local array and remote buffer)
    std::cout << "Creating local array";
    std::vector<unsigned char> localArray(dataSize, 0);
    std::cout << " and remote buffer.." << std::endl;
    cl::Buffer remoteBuffer(context, CL_MEM_WRITE_ONLY, dataSize);

    // Create program
    std::cout << "Creating program.." << std::endl;
    cl::Program program(context, util::loadProgram("opencl/fractal.cl"), true);
    /* cl::make_kernel<cl::Buffer> main(program, "fractalSingle"); */
    cl::make_kernel<cl::Buffer, unsigned int> main(program, "fractalBlock");

    for (unsigned repeat = 0; repeat < globalRepeats; repeat++) {
      // Enqueue
      /* std::cout << "Enqueueing.." << std::endl; */
      /* main(cl::EnqueueArgs(queue, cl::NDRange(workItemCount)), remoteBuffer); */
      main(cl::EnqueueArgs(queue, cl::NDRange(workItemCount)), remoteBuffer, workItemIterations);

      // Finishing
      /* std::cout << "Finishing.." << std::endl; */
      queue.finish();

      // Getting results
      /* std::cout << "Getting results.." << std::endl; */
      cl::copy(queue, remoteBuffer, localArray.begin(), localArray.end());

      /* for (size_t i = 0; i < localArray.size(); i++) { */
      /*   std::cout << static_cast<short>(localArray[i]) << ' '; */
      /* } */
    }

    std::cout << std::endl;

  } catch (const cl::Error &err) {
    std::cerr
      << "OpenCL error: "
      << err.what() << '(' << err.err() << ')'
      << std::endl;
    return 1;
  }
}
