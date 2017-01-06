#pragma once

#include <clest/cl.hpp>

namespace clest {

  class ClRunner {
  public:

    ClRunner(cl_device_type type,
             const std::vector<const char *> & requirements);

    void loadProgram(const std::string & path);

    static void printFull() {
      clest::println("Listing all platforms and devices..");
      try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        std::string name;

        if (platforms.empty()) {
          throw clest::Exception::build("OpenCL platforms not found.");
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
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }
    }

    static void printLongDeviceInfo(const cl::Device & device) {
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
        "Extensions:               {}\n"
        "=========================|\n",
        device.getInfo<CL_DEVICE_NAME>(),
        device.getInfo<CL_DRIVER_VERSION>(),
        device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(),
        device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>(),
        device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>(),
        device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>(),
        device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(),
        device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(),
        device.getInfo<CL_DEVICE_EXTENSIONS>()
      );
    }

  private:

    bool mContext = false;
    std::unique_ptr<cl::Program> uProgram;
    std::unique_ptr<std::vector<cl::Device>> uDevices;
    std::unique_ptr<cl::Context> uContext;
  };
}