#include <iostream>
#include <vector>
#include <string>

#define __CL_ENABLE_EXCEPTIONS
#include <cl.hpp>

int main() {
  try {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform;

    if (platforms.empty()) {
      std::cerr << "OpenCL platforms not found." << std::endl;
      return 1;
    }

    cl::Context context;
    std::vector<cl::Device> devices;
    for (auto p = platforms.begin(); devices.empty() && p != platforms.end(); p++) {
      std::vector<cl::Device> platformDevices;

      try {
        p->getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);

        for (auto device = platformDevices.begin(); devices.empty() && device != platformDevices.end(); device++) {
          std::string ext = device->getInfo<CL_DEVICE_EXTENSIONS>();

          if (ext.find("cl_khr_fp64") == std::string::npos) {
            std::cout << "cl_khr_fp64" << std::endl;
            if (ext.find("cl_amd_fp64") == std::string::npos) {
              std::cout << "cl_amd_fp64" << std::endl;
              std::cout << "Skipping: " << device->getInfo<CL_DEVICE_NAME>() << std::endl;
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

    std::cout << devices[0].getInfo<CL_DEVICE_NAME>() << std::endl;

  } catch (const cl::Error &err) {
    std::cerr
      << "OpenCL error: "
      << err.what() << '(' << err.err() << ')'
      << std::endl;
    return 1;
  }
}
