#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#define __CL_ENABLE_EXCEPTIONS
#include <cl/cl.hpp>
#include <clest/ostream.hpp>

#ifdef _CMAKE_BOOST_FOUND
#include <boost/algorithm/string.hpp>
#define _TRIMMER boost
#else
#include <clest/util.hpp>
#define _TRIMMER clest
#endif

namespace clest {

  class ClRunner {
  public:

    ClRunner(cl_device_type type,
             const std::vector<const char *> & requirements);

    void loadProgram(const std::string & name,
                     const std::string & path,
                     const char * defines = "");
    void releaseProgram(const std::string & name);

    std::vector<cl::CommandQueue> commandQueues(int deviceCount);
    void releaseQueues();

    cl::Kernel makeKernel(const std::string & program,
                          const std::string & kernelName);

    template <typename ... T>
    cl::make_kernel<T...> makeKernelFunctor(const std::string & program,
                                            const std::string & kernelName) {
      auto builtProgram = mPrograms.find(program);
      if (builtProgram == mPrograms.end()) {
        throw clest::Exception::build("No program named {} has been loaded yet",
                                      program);
      }

      try {
        return cl::make_kernel<T...>(builtProgram->second, kernelName);
      } catch (cl::Error & err) {
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }
    }

    template <typename ... Args>
    const cl::Buffer & createBuffer(const std::string & name,
                                    const Args & ... args) {
      if (mBuffers.find(name) != mBuffers.end()) {
        throw clest::Exception::build("Trying to create a buffer with an"
                                      "existing name");
      }

      try {
        return (mBuffers.emplace(name, cl::Buffer(mContext, args...))
                .first)->second;
      } catch (cl::Error & err) {
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }
    }

    const cl::Buffer & ClRunner::getBuffer(const std::string & name) const {
      return mBuffers.at(name);
    }
    void releaseBuffer(const std::string & name);

    const cl::Context & context() const {
      return mContext;
    }
    operator const cl::Context & () const {
      return mContext;
    }

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
          _TRIMMER::trim(name);
          clest::println("{}", name);
          std::vector<cl::Device> platformDevices;

          try {
            platform->getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);

            for (auto device = platformDevices.begin();
                 device != platformDevices.end();
                 device++) {
              name = device->getInfo <CL_DEVICE_NAME>();
              _TRIMMER::trim(name);
              clest::println(" {} {}",
                             '*',
                             name);
            }
          } catch (...) {
          }
        }
      } catch (const cl::Error & err) {
        throw clest::Exception::build("OpenCL error: {} ({})",
                                      err.what(),
                                      err.err());
      }
    }

  private:
    cl::Context mContext;
    std::vector<cl::Device> mDevices;
    std::unordered_map<std::string, cl::Program> mPrograms;
    std::vector<cl::CommandQueue> mCommands;
    std::unordered_map<std::string, cl::Buffer> mBuffers;
  };
}

#undef _TRIMMER