//FPGA Kernel
__kernel void increment (__global float *a, float *c, int N)
{
	for(int i = 0 ; i < N ; i++)
		a[i] = a[i] + c;
}