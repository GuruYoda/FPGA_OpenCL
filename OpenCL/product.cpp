#include <math.h>
#include <fstream>
#include <stdio.h>
#include <string>
#include <iostream>
#include <CL/cl.hpp>

static const cl_uint vectorSize = 4096; //must be evenly divisible by workSize
static const cl_uint workSize = 256;

#define EPSILON (1e-2f)

void checkErr(cl_int err, const char * name)
{
    if (err != CL_SUCCESS) {
        std::cerr << "ERROR: " << name
                 << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void print_platform_info(std::vector<cl::Platform>* PlatformList)
{
	uint num_platforms=PlatformList->size();
	std::cout << "Number of Platforms: "<<num_platforms<<"\n";
	//Grab Platform Info for each platform
	for (uint i=0; i<num_platforms; i++)
	{
		std::cout <<"Platform " << i <<": "<<PlatformList->at(i).getInfo<CL_PLATFORM_NAME>()<<"\n";
	}
	std::cout<<"\n";
}

uint get_platform_id_with_string(std::vector<cl::Platform>* PlatformList, const char * name)
{
	uint num_platforms=PlatformList->size();
	uint ret_value=-1;
	//Grab Platform Info for each platform
	for (uint i=0; i<num_platforms; i++)
	{
		std::basic_string<char> platform_name = PlatformList->at(i).getInfo<CL_PLATFORM_NAME>();
		if (platform_name.find(name)!=std::string::npos) {
				return i;
		}
	}
	return ret_value;
}

void print_device_info(std::vector<cl::Device>* DeviceList)
{
	uint num_devices=DeviceList->size();
	std::cout << "Number of Devices in Platform: "<<num_devices<<"\n";
	//Grab Device Info for each device
	for (uint i=0; i<num_devices; i++)
	{
		printf("Device Number: %d\n", i);
		std::cout << "Device Name: "<<DeviceList->at(i).getInfo<CL_DEVICE_NAME>()<<"\n";
		std::cout << "Is Device Available?: "<<DeviceList->at(i).getInfo<CL_DEVICE_AVAILABLE>()<<"\n";
		std::cout << "Device Max Compute Units: "<<DeviceList->at(i).getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()<<"\n";
		std::cout << "Device Max Work Item Dimensions: "<<DeviceList->at(i).getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>()<<"\n";
		std::cout << "Device Max Work Group Size: "<<DeviceList->at(i).getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()<<"\n";
		std::cout << "Device Max Frequency: "<<DeviceList->at(i).getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>()<<"\n";
		std::cout << "Device Max Mem Alloc Size: "<<DeviceList->at(i).getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>()<<"\n\n";
	}
}

void fill_generate(cl_float X[], cl_float Y[], cl_float Z[], cl_float LO, cl_float HI, size_t vectorSize)
{

	//Assigns randome number from LO to HI to all locatoin of X and Y
	for (uint i = 0; i < vectorSize; ++i) {
		X[i] =  LO + (cl_float)rand()/((cl_float)RAND_MAX/(HI-LO));
		Y[i] =  LO + (cl_float)rand()/((cl_float)RAND_MAX/(HI-LO));
	}
}

bool verification (float X[], float Y[], float Z[], float CalcZ[], size_t vectorSize)
{
	//Verify if OpenCL Calculation is Same as C Result
	for(uint i = 0; i < vectorSize-4; i++) {
		if(fabs(CalcZ[i] - Z[i]) > EPSILON) {
			 printf("\nVERIFICATION FAILED! index %d, X:%f, Y:%f, OpenCL Result:%f != Result %f)",
				  i, X[i], Y[i], Z[i], CalcZ[i]);
			 return false;
		}
    }

	// Print 10 Sample Data to Standard Out
	printf("\n\nVERIFICATION PASSED!!!\n\nSome Sample of Results\n");
	printf("------------------------------------\n");
	for (int i = 0; i < (int)vectorSize; i=i+((int)vectorSize)/5) {
		printf("Index %d: Input 1 is %f, Input 2 is %f, Result is %f\n", i, ((float*)X)[i], ((float*)Y)[i], ((float*)Z)[i]);
	}
	return true;
}

int main(void)
{

	void print_platform_info(std::vector<cl::Platform>* PlatformList);
	uint get_platform_id_with_string(std::vector<cl::Platform>*, const char * name);
	void print_device_info(std::vector<cl::Device>*);
	void fill_generate(cl_float X[], cl_float Y[], cl_float Z[], float LO, float HI, size_t vectorSize);
	bool verification (float X[], float Y[], float Z[], float CalcZ[], size_t vectorSize);
	void checkErr(cl_int err, const char * name);
	cl_int err;

	//Setup Platform
	//Get Platform ID
	std::vector<cl::Platform> PlatformList;
	err = cl::Platform::get(&PlatformList);
	checkErr(err, "Get Platform List");
	checkErr(PlatformList.size()>=1 ? CL_SUCCESS : -1, "cl::Platform::get");
	print_platform_info(&PlatformList);
	
	//Look for Fast Emulation Platform
	uint current_platform_id=get_platform_id_with_string(&PlatformList, "Emulation");
	printf("Using Platform: %d\n\n", current_platform_id);
	
	//Setup Device
	//Get Device ID
	std::vector<cl::Device> DeviceList;
	
	err = PlatformList[current_platform_id].getDevices(CL_DEVICE_TYPE_ALL, &DeviceList);
	checkErr(err, "Get Devices");
	print_device_info(&DeviceList);
	
	//Create Context
	cl::Context mycontext(DeviceList, NULL, NULL, NULL, &err);
	checkErr(err, "Context Constructor");
	
	//Create Command queue
	cl::CommandQueue myqueue(mycontext, DeviceList[0], 0, &err);
	checkErr(err, "Queue Constructor");
	
	//Create Buffers for input and output
	cl::Buffer Buffer_In(mycontext, CL_MEM_READ_ONLY, sizeof(cl_float)*vectorSize);
	cl::Buffer Buffer_In2(mycontext, CL_MEM_READ_ONLY, sizeof(cl_float)*vectorSize);
	cl::Buffer Buffer_Out(mycontext, CL_MEM_WRITE_ONLY, sizeof(cl_float)*vectorSize);
	
	//Inputs and Outputs to Kernel, X and Y are inputs, Z is output
	//The aligned attribute is used to ensure alignment
	//so that DMA could be used if we were working with a real FPGA board
	cl_float X[vectorSize]  __attribute__ ((aligned (64)));
	cl_float Y[vectorSize]  __attribute__ ((aligned (64)));
	cl_float Z[vectorSize]  __attribute__ ((aligned (64)));
	
	//Allocates memory with value from 0 to 1000
	cl_float LO= 0;   cl_float HI=1000;
	fill_generate(X, Y, Z, LO, HI, vectorSize);
	
	//Write data to device
	err = myqueue.enqueueWriteBuffer(Buffer_In, CL_FALSE, 0, sizeof(cl_float)*vectorSize, X);
	checkErr(err, "WriteBuffer");
	err = myqueue.enqueueWriteBuffer(Buffer_In2, CL_FALSE, 0, sizeof(cl_float)*vectorSize, Y);
	checkErr(err, "WriteBuffer 2");
	myqueue.finish();
	
	const char *kernel_name = "SimpleKernel";
	
	//Read in binaries from file
	std::ifstream aocx_stream("../SimpleKernel.aocx", std::ios::in|std::ios::binary);
	checkErr(aocx_stream.is_open() ? CL_SUCCESS:-1, "SimpleKernel.aocx");
	std::string prog(std::istreambuf_iterator<char>(aocx_stream), (std::istreambuf_iterator<char>()));
	cl::Program::Binaries mybinaries (1, std::make_pair(prog.c_str(), prog.length()));
	
	// Create the Program from the AOCX file.
	cl::Program program(mycontext, DeviceList, mybinaries, NULL, &err);
	checkErr(err, "Program Constructor");
	
	// build the program
	//////////////      Compile the Kernel.... For Intel FPGA, nothing is done here, but this conforms to the standard
	err=program.build(DeviceList);
	checkErr(err, "Build Program");
	
	// create the kernel
	//////////////       Find Kernel in Program
	cl::Kernel kernel(program, kernel_name, &err);
	checkErr(err, "Kernel Creation");

	//////////////     Set Arguments to the Kernels
	err = kernel.setArg(0, Buffer_In);
	checkErr(err, "Arg 0");
	err = kernel.setArg(1, Buffer_In2);
	checkErr(err, "Arg 1");
	err = kernel.setArg(2, Buffer_Out);
	checkErr(err, "Arg 2");
	//err = kernel.setArg(3, vectorSize);
	//checkErr(err, "Arg 3");
	
	printf("\nLaunching the kernel...\n");
	
	// Launch Kernel
	err=myqueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(vectorSize), cl::NDRange(workSize));
	//err=myqueue.enqueueTask(kernel);
	checkErr(err, "Kernel Execution");

	// read the output
	err=myqueue.enqueueReadBuffer(Buffer_Out, CL_TRUE, 0, sizeof(cl_float)*vectorSize, Z);
	checkErr(err, "Read Buffer");

	err=myqueue.finish();
	checkErr(err, "Finish Queue");
	
	float CalcZ[vectorSize];

	for (uint i=0; i<vectorSize; i++)
	{
		//////////////  Equivalent Code running on CPUs
		CalcZ[i] = X[i] * Y[i]; 
				
	}

	//Print Performance Results
	verification (X, Y, Z, CalcZ, vectorSize);
	
	return 0;
}