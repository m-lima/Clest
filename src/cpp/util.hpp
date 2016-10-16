#ifndef CLEST_UTIL_HPP
#define CLEST_UTIL_HPP

#include <string>
#include <fstream>

namespace clest {
  namespace util {

    bool isBigEndian() {
      union {
        uint32_t i;
        char c[4];
      } testValue = { 0x01020304 };

      return testValue.c[0] == 1;
    }

    bool listALL() {
      std::cout << "Listing all platforms and devices.." << std::endl;
      try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        std::string name;

        if (platforms.empty()) {
          std::cerr << "OpenCL platforms not found." << std::endl;
          return false;
        }

        cl::Context context;
        std::vector<cl::Device> devices;
        for (auto platform = platforms.begin();
            platform != platforms.end();
            platform++) {
          name = platform->getInfo<CL_PLATFORM_NAME>();
          boost::trim(name);
          std::cout << name << std::endl;
          std::vector<cl::Device> platformDevices;

          try {
            platform->getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);

            for (auto device = platformDevices.begin();
                device != platformDevices.end();
                device++) {
              name = device->getInfo < CL_DEVICE_NAME>();
              boost::trim(name);
              std::cout << ' ';
              if ((device + 1) != platformDevices.end()) {
                std::cout << "├";
              } else {
                std::cout << "└";
              }
              std::cout << ' ' << name << std::endl;
            }
          } catch (...) {
          }
        }
      } catch (const cl::Error &err) {
        std::cerr
          << "OpenCL error: "
          << err.what() << '(' << err.err() << ')'
          << std::endl;
        return false;
      }

      return true;
    }

    std::string loadProgram(std::string input) {
      std::ifstream stream(input.c_str());
      if (!stream.is_open()) {
        std::cerr << "Cannot open file: " << input << std::endl;
        exit(1);
      }

      return std::string(std::istreambuf_iterator<char>(stream),
          std::istreambuf_iterator<char>());
    }

    void populateDevices(std::vector<cl::Platform> * platforms,
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
  }
}

#endif //CLEST_UTIL_HPP
