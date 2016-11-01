#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <fmt/ostream.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <vector>

#include "util.hpp"

namespace {

  void _fractalSerial(std::vector<uint8_t> & data, uint8_t seed) {
    for (size_t i = 0; i < data.size(); ++i) {
      data[i] = static_cast<uint8_t>(i & ((i & seed) << 3));
    }
  }

}

int main(int argc, char *argv[]) {

  constexpr size_t VECTOR_SIZE = 0x100;

  if (argc > 3) {
    fmt::print(stderr,
               "Illegal number of parameters.\n"
               "Usage: clest [seed] [iterations]\n"
    );
    return 1;
  }

  short seed = 0;
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

  fmt::print("Using vector size: {}\n", VECTOR_SIZE);
  fmt::print("Using seed: {}\n", seed);
  fmt::print("Using iterations: {}\n", iterations);
  uint8_t reference = seed & 0xFF;

  std::vector<uint8_t> data;
  data.resize(VECTOR_SIZE);

  boost::posix_time::ptime start;
  boost::posix_time::time_duration duration;

  // Serial
  double avgSerial = 0;
  fmt::print("Starting serial battery [{}]\n\n",
             boost::posix_time::to_simple_string(
               boost::posix_time::microsec_clock::local_time()
             )
  );
  for (unsigned int i = 0; i < iterations; ++i) {
    start = boost::posix_time::microsec_clock::local_time();
    _fractalSerial(data, reference);
    duration = boost::posix_time::microsec_clock::local_time() - start;
    avgSerial += duration.total_microseconds();
    fmt::print(
      "Finished iteration [{}/{}] [{}ms]\n",
      i + 1,
      iterations,
      duration.total_microseconds() / 1000
    );
  }
  avgSerial /= iterations * 1000.0;

  fmt::print("Finished serial battery [{}]\n"
             "Average time: {:03.2f}ms\n\n",
             boost::posix_time::to_simple_string(
               boost::posix_time::microsec_clock::local_time()
             ),
             avgSerial
  );

  std::vector<uint8_t> data2;
  data2.resize(VECTOR_SIZE);

  // Parallel CPU
  double avgParallel = 0;
  fmt::print("Starting parallel battery [{}]\n\n",
             boost::posix_time::to_simple_string(
               boost::posix_time::microsec_clock::local_time()
             )
  );
  for (unsigned int i = 0; i < iterations; ++i) {
    start = boost::posix_time::microsec_clock::local_time();
    tbb::blocked_range<size_t> block(0, data2.size());
    tbb::parallel_for(block, [&](tbb::blocked_range<size_t> range) {
      for (size_t j = range.begin(); j != range.end(); ++j) {
        data2[j] = static_cast<uint8_t>(j & ((j & reference) << 3));
      }
    });
    duration = boost::posix_time::microsec_clock::local_time() - start;
    avgParallel += duration.total_microseconds();
    fmt::print(
      "Finished iteration [{}/{}] [{}ms]\n",
      i + 1,
      iterations,
      duration.total_microseconds() / 1000
    );
  }
  avgParallel /= iterations * 1000.0;

  fmt::print("Finished parallel battery [{}]\n"
             "Average time: {:03.2f}ms\n\n",
             boost::posix_time::to_simple_string(
               boost::posix_time::microsec_clock::local_time()
             ),
             avgParallel
  );

  fmt::print("Check results.. ");
  size_t errorCount = 0;
  for (size_t i = 0; i < data.size(); i++) {
    if (data[i] != data2[i]) {
      errorCount++;
    }
  }
  fmt::print("{} errors out of {}\n", errorCount, data.size());
  if (errorCount == 0) {
    data2 = std::vector<uint8_t>();
  }

  // Parallel GPU
  double avgCL = 0;
  fmt::print("Starting OpenCL battery [{}]\n\n",
             boost::posix_time::to_simple_string(
               boost::posix_time::microsec_clock::local_time()
             )
  );

  clest::util::listAll();

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
        }
        if (!devices.empty()) {
          fmt::print("Going for: {}\n", platform.getInfo<CL_PLATFORM_NAME>());
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

  } catch (const cl::Error & error) {
    fmt::print(stderr, "OpenCL error: {} ({})", error.what(), error.err());
    return 1;
  }
  
  return 0;
}
