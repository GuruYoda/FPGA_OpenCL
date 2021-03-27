//ACL Kernel
__kernel void helloWorld()
{
	printf("Hello world from thread %d\n", get_global_id(0));
}