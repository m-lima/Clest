#ifndef CLEST_UTIL_HPP
#define CLEST_UTIL_HPP

#include <string>
#include <fstream>

namespace clest {
  namespace util {
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
        for (auto platform = platforms.begin(); platform != platforms.end(); platform++) {
          name = platform->getInfo<CL_PLATFORM_NAME>();
          boost::trim(name);
          std::cout << name << std::endl;
          std::vector<cl::Device> platformDevices;

          try {
            platform->getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);

            for (auto device = platformDevices.begin(); device != platformDevices.end(); device++) {
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

      return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
    }
  }
}

#endif //CLEST_UTIL_HPP