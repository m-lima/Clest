#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include <cl.hpp>

#include "util.hpp"

using namespace clest;

int main() {
  if (!util::listALL()) {
    return 1;
  }

  std::cout << std::endl;
  std::cout << std::endl;

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

    for (auto device = devices.begin(); device != devices.end(); device++) {
      std::string name = device->getInfo<CL_DEVICE_NAME>();
      boost::trim(name);
      std::cout << std::setw(11) << "Acceptable:" << std::setw(5) << ' ' << name << std::endl;
    }

  } catch (const cl::Error &err) {
    std::cerr
      << "OpenCL error: "
      << err.what() << '(' << err.err() << ')'
      << std::endl;
    return 1;
  }
}
