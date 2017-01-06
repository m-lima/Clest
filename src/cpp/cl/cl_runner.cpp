#include "cl_runner.hpp"

#include <fstream>

namespace clest {

  ClRunner::ClRunner(cl_device_type type,
                     const std::vector<const char *> & requirements) {
    try {
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
    } catch (cl::Error & err) {
      throw clest::Exception::build("OpenCL error: {} ({})",
                                    err.what(),
                                    err.err());
    }
  }

  void ClRunner::loadProgram(const std::string & name,
                             const std::string & path) {
    if (uContext == nullptr || uDevices == nullptr || uDevices->empty()) {
      throw clest::Exception::build("Trying to load program without a context "
                                    "or devices");
    }

    if (mPrograms.find(name) != mPrograms.end()) {
      throw clest::Exception::build("Trying to create a program with an"
                                    "existing name");
    }

    std::ifstream stream(path);

    if (!stream.is_open()) {
      throw clest::Exception::build("Could not load program:\n{}", path);
    }

    try {
      auto program = cl::Program(
        *uContext,
        std::string(std::istreambuf_iterator<char>(stream),
                    std::istreambuf_iterator<char>()));
      try {
        program.build(*uDevices);

        mPrograms[name] = std::move(program);
      } catch (cl::Error & err) {
        if (err.err() == CL_INVALID_PROGRAM_EXECUTABLE
            || err.err() == CL_BUILD_ERROR
            || err.err() == CL_BUILD_PROGRAM_FAILURE) {
          for (auto device : *uDevices) {
            clest::println(stderr,
                           "Build failure\n{}",
                           program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
          }
        }
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }
    } catch (cl::Error & err) {
      throw clest::Exception::build("OpenCL error: {} ({})",
                                    err.what(),
                                    err.err());
    }

    stream.close();
  }
}
