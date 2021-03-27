#include <iostream>
#include <vector>

#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 110

#include <CL/cl2.hpp>

int main ()
{
	//get platforms
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.empty())
	{
		std::cout << "*********** No platforms found! Aborting...!" << std::endl;
		return 1;
	}

	std::cout << "*********** Listing available platforms:" << std::endl;
	for (size_t i = 0; i < platforms.size(); ++i)
		std::cout << "platform[" << i << "]: " << platforms[i].getInfo<CL_PLATFORM_NAME>() << std::endl;

	cl::Platform selectedPlatform = platforms[0];
	std::cout << "*********** Using the following platform: " << selectedPlatform.getInfo<CL_PLATFORM_NAME>() << std::endl;

	//get devices
	std::vector<cl::Device> devices;
	selectedPlatform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
	if (devices.empty())
	{
		std::cout << "*********** No devices found on platform " << selectedPlatform.getInfo<CL_PLATFORM_NAME>()
			<<"! Aborting...!" << std::endl;
		return 1;
	}

	std::cout << "*********** Listing available devices:" << std::endl;
	for (size_t i = 0; i < devices.size(); ++i)
		std::cout << "device[" << i << "]: " << devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;

	cl::Device selectedDevice = devices[0];
	std::cout << "*********** Using the following device: " << selectedDevice.getInfo<CL_DEVICE_NAME>() << std::endl;

	cl::Context context({selectedDevice});

	cl::Program::Sources sources;
	std::string kernelCode =
		"void kernel hello_world() "
		"{"
		"	printf(\"Hello world from thread %d\\n\", get_global_id(0));"
		"}";

	sources.push_back({kernelCode.c_str(), kernelCode.length()});

	cl::Program clProgram(context, sources);
	if (clProgram.build({selectedDevice}) != CL_SUCCESS)
	{
		std::cout<<"Building error: " << clProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(selectedDevice) << std::endl;
        return 1;
	}

	cl::CommandQueue queue(context, selectedDevice);

	cl::Kernel kernel_helloWorld(clProgram, "hello_world");

    queue.enqueueNDRangeKernel(kernel_helloWorld, cl::NullRange, cl::NDRange(20), cl::NullRange);
    queue.finish();

	return 0;
}