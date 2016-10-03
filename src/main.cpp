#include <iostream>
#include <vector>
#include <string>

#define __CL_ENABLE_EXCEPTIONS
#include <cl.hpp>

bool listALL() {
	try {
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		if (platforms.empty()) {
			std::cerr << "OpenCL platforms not found." << std::endl;
			return false;
		}

		cl::Context context;
		std::vector<cl::Device> devices;
		for (auto platform = platforms.begin(); platform != platforms.end(); platform++) {
			std::cout << "Platform:\t\t\t" << platform->getInfo<CL_PLATFORM_NAME>() << std::endl;
			std::vector<cl::Device> platformDevices;

			try {
				platform->getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);

				for (auto device = platformDevices.begin(); device != platformDevices.end(); device++) {
					std::cout << "\tDevice:\t\t\t" << device->getInfo<CL_DEVICE_NAME>() << std::endl;
				}
			}
			catch (...) {
			}
		}
	}
	catch (const cl::Error &err) {
		std::cerr
			<< "OpenCL error: "
			<< err.what() << '(' << err.err() << ')'
			<< std::endl;
		return false;
	}

	return true;
}

int main() {
  listALL();
  try {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
      std::cerr << "OpenCL platforms not found." << std::endl;
      return 1;
    }

    cl::Context context;
    std::vector<cl::Device> devices;
    for (auto platform = platforms.begin(); devices.empty() && platform != platforms.end(); platform++) {
      std::vector<cl::Device> platformDevices;

      try {
        platform->getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);

        for (auto device = platformDevices.begin(); devices.empty() && device != platformDevices.end(); device++) {
          std::string ext = device->getInfo<CL_DEVICE_EXTENSIONS>();

          if (ext.find("cl_khr_fp64") == std::string::npos) {
            if (ext.find("cl_amd_fp64") == std::string::npos) {
              continue;
            }
          }

          devices.push_back(*device);
          context = cl::Context(devices);
        }

      } catch (...) {
        devices.clear();
      }
    }

    if (devices.empty()) {
      std::cerr << "GPUs with double precision not found." << std::endl;
      return 1;
    }

    for (auto device = devices.begin(); device != devices.end(); device++) {
      std::cout << "Acceptable: " << device->getInfo<CL_DEVICE_NAME>() << std::endl;
    }

  } catch (const cl::Error &err) {
    std::cerr
      << "OpenCL error: "
      << err.what() << '(' << err.err() << ')'
      << std::endl;
    return 1;
  }
}
