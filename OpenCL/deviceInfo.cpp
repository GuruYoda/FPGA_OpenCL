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

	//cl::Program::Sources sources;
	std::cout << "Reading bianaries... \n ";
	size_t lengths[1];
	unsigned char* binaries[1] ={NULL};
	cl_int status[1];
	cl_int error;
	cl_program program;
	const char options[] = "";
	FILE *fp = fopen("helloWorld.aocx","rb");
	fseek(fp,0,SEEK_END);
	lengths[0] = ftell(fp);
	binaries[0] = (unsigned char*)malloc(sizeof(unsigned char)*lengths[0]);
	rewind(fp);
	fread(binaries[0],lengths[0],1,fp);
	fclose(fp);
	program = clCreateProgramWithBinary(context,
	 1,
	 device_list,
	 lengths,
	 (const unsigned char **)binaries,
	 status,
	 &error);
	clBuildProgram(program,1,device_list,options,NULL,NULL);
	
	cl::CommandQueue queue(context, selectedDevice);

	cl_kernel clCreateKernel (clProgram program, const char *helloWorld, NULL);

    queue.enqueueNDRangeKernel(cl_command_queue queue, cl_kernel helloWorld, cl::NullRange, cl::NDRange(20), cl::NullRange);
    queue.finish();
	
	return 0;
}