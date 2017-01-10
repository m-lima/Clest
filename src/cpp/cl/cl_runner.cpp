#include "cl_runner.hpp"

#include <fstream>
#include <numeric>

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
      size_t bestCount = 0;
      int bestIndex = 0;
      for (int i = 0; i < platforms.size(); ++i) {
        platforms[i].getDevices(type, &platformDevices);
        auto deviceCount = platformDevices.size();

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

      mDevices.reserve(bestCount);
      if (requirements.empty()) {
        platforms[bestIndex].getDevices(type, &mDevices);
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
            mDevices.push_back(device);
            printLongDeviceInfo(device);
          }
        }
      }

      mContext = cl::Context(mDevices);
    } catch (cl::Error & err) {
      throw clest::Exception::build("OpenCL error: {} ({})",
                                    err.what(),
                                    err.err());
    }
  }

  void ClRunner::loadProgram(const std::string & name,
                             const std::string & path,
                             const char * defines)
  {
    if (mDevices.empty()) {
      throw clest::Exception::build("Trying to load program without devices");
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
        mContext,
        std::string(std::istreambuf_iterator<char>(stream),
                    std::istreambuf_iterator<char>()));

      clest::println("Build log for {} ({})", name, path);
      for (auto device : mDevices) {
        clest::println("== Device {}:\n{}",
                       device.getInfo<CL_DEVICE_NAME>(),
                       program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
      }
      try {
//#if defined(_DEBUG) || defined(NDEBUG)
        //const char * options = defines + " -cl-opt-disable";
        //program.build(mDevices, options);
//#else
        program.build(mDevices, defines);
//#endif

        mPrograms[name] = std::move(program);
      } catch (cl::Error & err) {
        if (err.err() == CL_INVALID_PROGRAM_EXECUTABLE
            || err.err() == CL_BUILD_ERROR
            || err.err() == CL_BUILD_PROGRAM_FAILURE) {
          for (auto device : mDevices) {
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

  void ClRunner::releaseProgram(const std::string & name) {
    mPrograms.erase(name);
  }

  std::vector<cl::CommandQueue> ClRunner::commandQueues(int deviceCount) {
    if (deviceCount <= 0) {
      return std::vector<cl::CommandQueue>(0);
    }

    if (mCommands.size() < deviceCount) {
      mCommands.reserve(deviceCount);
      
      try {
        for (size_t i = mCommands.size(); i < deviceCount; ++i) {
          mCommands.emplace_back(mContext, mDevices[i]);
        }
      } catch (cl::Error & err) {
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }

      if (mCommands.size() < deviceCount) {
        throw clest::Exception::build("Could not create command queues");
      }
    }

    return std::vector<cl::CommandQueue>(mCommands.cbegin(),
                                         mCommands.cbegin() + deviceCount);
  }

  void ClRunner::releaseQueues() {
    mCommands = std::vector<cl::CommandQueue>(0);
  }

  cl::Kernel ClRunner::makeKernel(const std::string & program,
                                  const std::string & kernelName) {

    auto builtProgram = mPrograms.find(program);
    if (builtProgram == mPrograms.end()) {
      throw clest::Exception::build("No program named {} has been loaded yet",
                                    program);
    }

    cl::Kernel kernel(builtProgram->second, kernelName.c_str());

    for (auto device : mDevices) {
      clest::println("Kernel info for {}", device.getInfo<CL_DEVICE_NAME>());
      auto compileWorkGroupSize =
        kernel.getWorkGroupInfo<CL_KERNEL_COMPILE_WORK_GROUP_SIZE>(device);
      clest::println(" * Compile work group size:        {}, {}, {}",
                     compileWorkGroupSize[0],
                     compileWorkGroupSize[1],
                     compileWorkGroupSize[2]);
      uint16_t globalSize[3];
      size_t yo;
      clGetKernelWorkGroupInfo(kernel(),
                               device(),
                               CL_KERNEL_GLOBAL_WORK_SIZE,
                               sizeof(globalSize),
                               globalSize,
                               &yo);
      clest::println(" * Global work size:               {}, {}, {} - {}",
                     globalSize[0],
                     globalSize[1],
                     globalSize[2], yo);
      clest::println(" * Local memory size:              {}",
                     kernel.getWorkGroupInfo
                     <CL_KERNEL_LOCAL_MEM_SIZE>(device));
      clest::println(" * Preferred group size multiple:  {}",
                     kernel.getWorkGroupInfo
                     <CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device));
      clest::println(" * Private memory size:            {}",
                     kernel.getWorkGroupInfo
                     <CL_KERNEL_PRIVATE_MEM_SIZE>(device));
      clest::println(" * Work group size:                {}",
                     kernel.getWorkGroupInfo
                     <CL_KERNEL_WORK_GROUP_SIZE>(device));
    }
  }

  void ClRunner::releaseBuffer(const std::string & name) {
    mBuffers.erase(name);
  }
}
