#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "ostream.hpp"

namespace clest {
  inline bool listAll() {
    clest::println("Listing all platforms and devices..");
    try {
      std::vector<cl::Platform> platforms;
      cl::Platform::get(&platforms);
      std::string name;

      if (platforms.empty()) {
        clest::println(stderr, "OpenCL platforms not found.");
        return false;
      }

      cl::Context context;
      std::vector<cl::Device> devices;
      for (auto platform = platforms.begin();
           platform != platforms.end();
           platform++) {
        name = platform->getInfo<CL_PLATFORM_NAME>();
        boost::trim(name);
        clest::println("{}", name);
        std::vector<cl::Device> platformDevices;

        try {
          platform->getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);

          for (auto device = platformDevices.begin();
               device != platformDevices.end();
               device++) {
            name = device->getInfo <CL_DEVICE_NAME>();
            boost::trim(name);
            clest::println(" {} {}",
              ((device + 1) != platformDevices.end() ? "├" : "└"),
                       name);
          }
        } catch (...) {
        }
      }
    } catch (const cl::Error &err) {
      clest::println(stderr, "OpenCL error: {} ({})", err.what(), err.err());
      return false;
    }

    return true;
  }

  inline std::string loadProgram(std::string input) {
    std::ifstream stream(input.c_str());
    if (!stream.is_open()) {
      clest::println("Cannot open file: {}", input);
      exit(1);
    }

    return std::string(std::istreambuf_iterator<char>(stream),
                       std::istreambuf_iterator<char>());
  }

  inline void populateDevices(std::vector<cl::Platform> * platforms,
                              std::vector<cl::Device> * devices,
                              cl_device_type deviceType) {
    for (auto platform = platforms->begin();
         devices->empty() && platform != platforms->end();
         platform++) {
      std::vector<cl::Device> platformDevices;

      try {
        platform->getDevices(deviceType, &platformDevices);

        for (auto device = platformDevices.begin();
             devices->empty() && device != platformDevices.end();
             device++) {
          std::string ext = device->getInfo<CL_DEVICE_EXTENSIONS>();

          if (ext.find("cl_khr_fp64") == std::string::npos) {
            if (ext.find("cl_amd_fp64") == std::string::npos) {
              continue;
            }
          }

          devices->push_back(*device);
        }

      } catch (...) {
        devices->clear();
      }
    }
  }

  inline void printLongDeviceInfo(const cl::Device & device) {
    clest::println(
      "=========================|\n"
      "Name:                     {}\n"
      "OpenCL version:           {}\n"
      "Global memory size:       {}MB\n"
      "Local memory size:        {}KB\n"
      "Max constant buffer size: {}KB\n"
      "Max clock frequency:      {}Hz\n"
      "Max compute units:        {}\n"
      "Max work group size:      {}\n"
      "=========================|\n",
      device.getInfo<CL_DEVICE_NAME>(),
      device.getInfo<CL_DRIVER_VERSION>(),
      device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(),
      device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>(),
      device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>(),
      device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>(),
      device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(),
      device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()
    );
  }
}