#include <iostream>
#include <vector>
#include <string>

/* #define CL_HPP_ENABLE_EXCEPTIONS */
/* #define CL_HPP_TARGET_OPENCL_VERSION 200 */

/* #ifdef __APPLE__ */
/* #include <OpenCL/cl.h> */
/* #else */
/* #include <CL/cl.h> */
/* #endif */

#define __CL_ENABLE_EXCEPTIONS
#include <cl.hpp>

int main() {
  try {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform;

    if (platforms.empty()) {
      std::cerr << "OpenCL platforms not found." << std::endl;
      return 1;
    }

  } catch (const cl::Error &err) {
    std::cerr
      << "OpenCL error: "
      << err.what() << '(' << err.err() << ')'
      << std::endl;
    return 1;
  }
}
