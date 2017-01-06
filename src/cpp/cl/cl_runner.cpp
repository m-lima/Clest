#include "cl_runner.hpp"

namespace clest {

  ClRunner::ClRunner(cl_device_type type,
                     const std::vector<const char *> & requirements) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
      throw clest::Exception::build("OpenCL platforms not found");
    }

    clest::println("Detecting best platform..");
    std::vector<cl::Device> platformDevices;
    unsigned short bestCount = 0;
    int bestIndex = 0;
    for (int i = 0; i < platforms.size(); ++i) {
      platforms[i].getDevices(type, &platformDevices);
      unsigned short deviceCount = platformDevices.size();

      if (!requirements.empty()) {
        for (auto device : platformDevices) {
          std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
          for (const char * requirement : requirements) {
            if (extensions.find(requirement) == std::string::npos) {
              deviceCount--;
              break;
            }
          }
        }
      }

      if (deviceCount > bestCount) {
        bestCount = deviceCount;
        bestIndex = i;
      }
    }
    
    if (bestCount == 0) {
      throw clest::Exception::build("No compatible OpenCL device found");
    }

    clest::println("Chose {} with {} compatible device{}",
                   platforms[bestIndex].getInfo<CL_PLATFORM_NAME>(),
                   bestCount,
                   bestCount > 1 ? 's' : ' ');

    uDevices = std::make_unique<std::vector<cl::Device>>();
    uDevices->reserve(bestCount);
    if (requirements.empty()) {
      platforms[bestIndex].getDevices(type, uDevices.get());
    } else {
      platforms[bestIndex].getDevices(type, &platformDevices);
      for (auto device : platformDevices) {
        std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
        bool compatible = true;
        for (const char * requirement : requirements) {
          if (extensions.find(requirement) == std::string::npos) {
            compatible = false;
            break;
          }
        }
        if (compatible) {
          uDevices->push_back(device);
          printLongDeviceInfo(device);
        }
      }
    }

    uContext = std::make_unique<cl::Context>(*uDevices);
  }

  void ClRunner::loadProgram(const std::string & path) {
    if (uContext == nullptr) {
      throw clest::Exception::build("Trying to load program without a context");
    }

    std::ifstream stream(path);

    if (!stream.is_open()) {
      throw clest::Exception::build("Could not load program:\n{}", path);
    }

    uProgram = std::make_unique<cl::Program>(
      *uContext,
      std::string(std::istreambuf_iterator<char>(stream),
                  std::istreambuf_iterator<char>()));

    uProgram->build(*uDevices);

    stream.close();
  }
}
