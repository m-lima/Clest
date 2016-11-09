#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <fmt/ostream.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <vector>

#include "util.hpp"

namespace {

  void _fractalSerialS(std::vector<uint8_t> & data) {
    for (size_t i = 0; i < data.size(); ++i) {
      data[i] = static_cast<uint8_t>(i & (i >> 8));
    }
  }

  void _fractalSerialP(std::vector<uint8_t> & data, uint8_t seed) {
    for (size_t i = 0; i < data.size(); ++i) {
      uint8_t base = static_cast<uint8_t>(i & (i >> 8));
      uint8_t value = base;
      for (uint8_t j = 0; j < seed; ++j) {
        value *= base;
      }
      data[i] = value;
    }
  }

}

int main(int argc, char *argv[]) {

  constexpr size_t VECTOR_SIZE = 0x20000000;

  {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
      fmt::print(stderr, "OpenCL platforms not found\n");
      return 1;
    }

    std::vector<cl::Device> platformDevices;
    for (auto & platform : platforms) {
      try {
        platform.getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);
        for (auto & device : platformDevices) {
          clest::util::printLongDeviceInfo(device);
        }
      } catch (...) {
      }
    }
  }

  if (argc > 3) {
    fmt::print(stderr,
               "Illegal number of parameters.\n"
               "Usage: clest [seed] [iterations]\n"
    );
    return 1;
  }

  short seed = 12;
  unsigned int iterations = 25;

  if (argc > 1) {
    std::istringstream iss(argv[1]);
    if (!(iss >> seed)) {
      fmt::print(stderr, "Invalid seed value: {}\n", argv[1]);
    }

    if (argc > 2) {
      iss = std::istringstream(argv[2]);
      if (!(iss >> iterations)) {
        fmt::print(stderr, "Invalid iteration number: {}\n", argv[2]);
      }
    }
  }

  fmt::print(stderr, "Using vector size: {}\n", VECTOR_SIZE);
  fmt::print(stderr, "Using seed: {}\n", seed);
  fmt::print(stderr, "Using iterations: {}\n", iterations);
  fmt::print("Using {} iterations\n", iterations);
  uint8_t reference = seed & 0xFF;

  double avgSerial;
  double avgParallel;
  double avgOCL;

  std::vector<uint8_t> dataSerial;
  std::vector<uint8_t> dataParallel;
  std::vector<uint8_t> dataOCL;

  boost::posix_time::ptime start;
  boost::posix_time::time_duration duration;

  for (uint8_t simple = 0; simple < 2; simple++) {
    for (size_t size = 1; size <= VECTOR_SIZE; size <<= 1) {
      avgSerial = 0;
      avgParallel = 0;
      avgOCL = 0;

      fmt::print(stderr, "======================== SIZE: ({})\n\n", size);
      dataSerial.resize(size);
      dataParallel.resize(size);
      dataOCL.resize(size);

      // Serial
      fmt::print(
        stderr,
        "Starting serial battery [{}]\n",
        boost::posix_time::to_simple_string(
          boost::posix_time::microsec_clock::local_time()
        )
      );
      for (unsigned int i = 0; i < iterations; ++i) {
        start = boost::posix_time::microsec_clock::local_time();
        if (simple == 0) {
          _fractalSerialS(dataSerial);
        } else {
          _fractalSerialP(dataSerial, reference);
        }
        duration = boost::posix_time::microsec_clock::local_time() - start;
        avgSerial += duration.total_microseconds();
        fmt::print(
          stderr,
          "Finished iteration [{}/{}] [{}ms]\n",
          i + 1,
          iterations,
          duration.total_microseconds() / 1000
        );
      }
      avgSerial /= iterations * 1000.0;

      fmt::print(
        stderr,
        "Finished serial battery [{}]\n"
        "Average time: {:03.2f}ms\n\n",
        boost::posix_time::to_simple_string(
          boost::posix_time::microsec_clock::local_time()
        ),
        avgSerial
      );

      // Parallel CPU
      fmt::print(
        stderr,
        "Starting parallel battery [{}]\n",
        boost::posix_time::to_simple_string(
          boost::posix_time::microsec_clock::local_time()
        )
      );
      for (unsigned int i = 0; i < iterations; ++i) {
        start = boost::posix_time::microsec_clock::local_time();
        tbb::blocked_range<size_t> block(0, dataParallel.size());
        if (simple == 0) {
          tbb::parallel_for(block, [&](tbb::blocked_range<size_t> range) {
            for (size_t j = range.begin(); j != range.end(); ++j) {
              dataParallel[j] = static_cast<uint8_t>(j & (j >> 8));
            }
          });
        } else {
          tbb::parallel_for(block, [&](tbb::blocked_range<size_t> range) {
            for (size_t j = range.begin(); j != range.end(); ++j) {
              uint8_t base = static_cast<uint8_t>(j & (j >> 8));
              uint8_t value = base;
              for (uint8_t k = 0; k < reference; ++k) {
                value *= base;
              }
              dataParallel[j] = value;
            }
          });
        }
        duration = boost::posix_time::microsec_clock::local_time() - start;
        avgParallel += duration.total_microseconds();
        fmt::print(
          stderr,
          "Finished iteration [{}/{}] [{}ms]\n",
          i + 1,
          iterations,
          duration.total_microseconds() / 1000
        );
      }
      avgParallel /= iterations * 1000.0;

      fmt::print(
        stderr,
        "Finished parallel battery [{}]\n"
        "Average time: {:03.2f}ms\n",
        boost::posix_time::to_simple_string(
          boost::posix_time::microsec_clock::local_time()
        ),
        avgParallel
      );

      fmt::print(stderr, "Check results.. ");
      size_t errorCount = 0;
      for (size_t i = 0; i < dataSerial.size(); i++) {
        if (dataSerial[i] != dataParallel[i]) {
          errorCount++;
        }
      }

      fmt::print(
        stderr,
        "{} errors out of {}\n\n",
        errorCount, dataSerial.size()
      );
      if (errorCount > 0) {
        fmt::print(stderr, "**** FAILED ****\n\n");
        avgParallel = -1;
      }
      dataParallel = std::vector<uint8_t>();

      // Parallel GPU
      fmt::print(
        stderr,
        "Starting OpenCL battery [{}]\n",
        boost::posix_time::to_simple_string(
          boost::posix_time::microsec_clock::local_time()
        )
      );

      std::unique_ptr<cl::Device> devicePtr = nullptr;
      std::unique_ptr<cl::Program> programPtr = nullptr;

      try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.empty()) {
          fmt::print(stderr, "OpenCL platforms not found\n");
          return 1;
        }

        std::vector<cl::Device> devices;
        std::vector<cl::Device> platformDevices;
        for (auto & platform : platforms) {
          try {
            platform.getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);
            for (auto & device : platformDevices) {
              devices.push_back(device);
              break;
            }
            if (!devices.empty()) {
              break;
            }
          } catch (...) {
            devices.clear();
          }
        }

        if (devices.empty()) {
          fmt::print(stderr, "No devices found\n");
          return 1;
        }

        cl::Context context(devices);
        devicePtr = std::make_unique<cl::Device>(devices[0]);
        cl::CommandQueue queue(context, *devicePtr);

        programPtr = std::make_unique<cl::Program>(
          context,
          clest::util::loadProgram("opencl/fractal.cl")
          );

        programPtr->build(devices);

        cl::Buffer remoteData(context, CL_MEM_WRITE_ONLY, size);

        if (simple == 0) {
          cl::make_kernel<cl::Buffer> kernel(
            *programPtr,
            "fractalS"
          );

          for (unsigned int i = 0; i < iterations; ++i) {
            start = boost::posix_time::microsec_clock::local_time();
            kernel(cl::EnqueueArgs(queue, cl::NDRange(size)), remoteData);

            queue.finish();

            cl::copy(queue, remoteData, dataOCL.begin(), dataOCL.end());
            duration = boost::posix_time::microsec_clock::local_time() - start;
            avgOCL += duration.total_microseconds();
            fmt::print(
              stderr,
              "Finished iteration [{}/{}] [{}ms]\n",
              i + 1,
              iterations,
              duration.total_microseconds() / 1000
            );
          }
        } else {
          cl::make_kernel<cl::Buffer, unsigned char> kernel(
            *programPtr,
            "fractalP"
          );

          for (unsigned int i = 0; i < iterations; ++i) {
            start = boost::posix_time::microsec_clock::local_time();
            kernel(
              cl::EnqueueArgs(queue, cl::NDRange(size)), remoteData, reference
            );

            queue.finish();

            cl::copy(queue, remoteData, dataOCL.begin(), dataOCL.end());
            duration = boost::posix_time::microsec_clock::local_time() - start;
            avgOCL += duration.total_microseconds();
            fmt::print(
              stderr,
              "Finished iteration [{}/{}] [{}ms]\n",
              i + 1,
              iterations,
              duration.total_microseconds() / 1000
            );
          }
        }

        avgOCL /= iterations * 1000.0;

        fmt::print(
          stderr,
          "Finished parallel battery [{}]\n"
          "Average time: {:03.2f}ms\n",
          boost::posix_time::to_simple_string(
            boost::posix_time::microsec_clock::local_time()
          ),
          avgOCL
        );

        fmt::print(stderr, "Check results.. ");
        size_t errorCount = 0;
        for (size_t i = 0; i < dataSerial.size(); i++) {
          if (dataSerial[i] != dataOCL[i]) {
            errorCount++;
          }
        }

        fmt::print(stderr, "{} errors out of {}\n\n", errorCount, dataSerial.size());
        if (errorCount > 0) {
          fmt::print(stderr, "**** FAILED ****\n\n");
          avgOCL = -1;
        }
        dataOCL = std::vector<uint8_t>();

      } catch (const cl::Error & error) {
        fmt::print(stderr, "OpenCL error: {} ({})\n", error.what(), error.err());
        switch (error.err()) {
          case CL_INVALID_PROGRAM_EXECUTABLE:
          case CL_BUILD_PROGRAM_FAILURE:
            if (programPtr == nullptr) {
              fmt::print(stderr, "Program pointer is NULL\n");
            } else {
              if (devicePtr == nullptr) {
                fmt::print(stderr, "Device pointer is NULL\n");
              }
              fmt::print(stderr,
                         "Build failure:\n{}\n",
                         programPtr->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*devicePtr));
            }
            break;
        }
        return 1;
      }

      fmt::print(
        "{} {:03.2f} {:03.2f} {:03.2f}\n",
        size,
        avgSerial,
        avgParallel,
        avgOCL
      );
    }
  }

  return 0;
}