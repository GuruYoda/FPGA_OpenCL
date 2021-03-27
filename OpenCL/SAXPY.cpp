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
		
		// Length of vectors
		unsigned int n = 100000;
		
		// Host input vectors
		double *h_a;
		double *h_b;
		// Host output vector
		double *h_c;

		// Device input buffers
		cl_mem d_a;
		cl_mem d_b;
		// Device output buffer
		cl_mem d_c;
			
		cl_platform_id cpPlatform;        // OpenCL platform
		cl_device_id device_id;           // device ID
		cl_context context;               // context
		cl_command_queue queue;           // command queue
		cl_program program;               // program
		cl_kernel kernel;                 // kernel
			
		// Size, in bytes, of each vector
		size_t bytes = n*sizeof(double);
		 
		// Allocate memory for each vector on host
		h_a = (double*)malloc(bytes);
		h_b = (double*)malloc(bytes);
		h_c = (double*)malloc(bytes);
			
		// Initialize vectors on host
		int i;
		for( i = 0; i < n; i++ )
		{
			h_a[i] = sinf(i)*sinf(i);
			h_b[i] = cosf(i)*cosf(i);
		}
 
		size_t globalSize, localSize;
		cl_int err;
			
		// Number of work items in each local work group
		localSize = 64;
		 
		// Number of total work items - localSize must be devisor
		globalSize = ceil(n/(float)localSize)*localSize;
		 
		// Bind to platform
		err = clGetPlatformIDs(1, &cpPlatform, NULL);
		 
		// Get ID for the device
		err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
		 
		// Create a context 
		context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
		 
		// Create a command queue
		queue = clCreateCommandQueue(context, device_id, 0, &err);
		 
		// Create the compute program from the source buffer
		program = clCreateProgramWithSource(context, 1, (const char **) & kernelSource, NULL, &err);
		 
		// Build the program executable
		clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
		 
		// Create the compute kernel in the program we wish to run
		kernel = clCreateKernel(program, "vecAdd", &err);
		 
		// Create the input and output arrays in device memory for our calculation
		d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
		d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
		d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);
		 
		// Write our data set into the input array in device memory
		err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, bytes, h_a, 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0, bytes, h_b, 0, NULL, NULL);
		 
		// Set the arguments to our compute kernel
		err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
		err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
		err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
		err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &n);
		 
		// Execute the kernel over the entire range of the data set 
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
		 
		// Wait for the command queue to get serviced before reading back results
		clFinish(queue);
		 
		// Read the results from the device
		clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0, bytes, h_c, 0, NULL, NULL );
		 
		//Sum up vector c and print result divided by n, this should equal 1 within error
		double sum = 0;
		for(i=0; i<n; i++)
			sum += h_c[i];
		printf("final result: %f\n", sum/n);
		 
		// release OpenCL resources
		clReleaseMemObject(d_a);
		clReleaseMemObject(d_b);
		clReleaseMemObject(d_c);
		clReleaseProgram(program);
		clReleaseKernel(kernel);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);
		 
		//release host memory
		free(h_a);
		free(h_b);
		free(h_c);
	
	return 0;

}

