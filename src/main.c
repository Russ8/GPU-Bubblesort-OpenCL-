
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)

clock_t timer;

//function that implements the power function
int pow(int a, int n) {
	int i;
	int temp = a;
	for (i = 0; i < n; i++) {
		a = a * temp;
	}
	return a;
}

int main(int argc, char** argv) {


	//setup the list size, which is a power of 2
	int i;
	int LIST_SIZE = pow(2, 16);

	//get list size from command line
	if(argc > 1) {
		LIST_SIZE = pow(2, atoi(argv[1]));
	}

	printf("LIST SIZE IS %i \n", LIST_SIZE);

	//allocate local buffer A
	int *A = (int*)malloc(sizeof(int)*LIST_SIZE);

	//set rand array values
	for (i = 0; i < LIST_SIZE; i++) {
		A[i] = rand() % LIST_SIZE;
	}

	//perform sequential bubble sort
	int index_a = 0;
	int index_b = 0;
	
	//sequential timer
	timer = clock();	


	int temp;
	//test bubble sort cpu
	for (index_a = 0; index_a < LIST_SIZE - 1; index_a++) {
		// cycle through the array LIST_SIZE - 1 times  
		for (index_b = 0; index_b < LIST_SIZE - index_a - 1; index_b++) {

			//compare index_b and index_b + 1 elements
			if (A[index_b] > A[index_b + 1]) {
				//swap
				temp = A[index_b];
				A[index_b] = A[index_b + 1];
				A[index_b + 1] = temp;
			}
		}
	}
	
	//print seq result
	int j;
	//for (j = 0; j < LIST_SIZE; j++)
	//	printf("%d ", A[j]);

	//set rand array values
	for (i = 0; i < LIST_SIZE; i++) {
		A[i] = rand() % LIST_SIZE;
	}	
	//print timer for seqential version
	timer = clock() - timer;
	printf("SEQ RUNTIME (ms): %li\n", timer * 1000 / CLOCKS_PER_SEC);

	//start time for GPU bubble sort
	timer = clock();
	int order = 0;

	//read kernel.cl file into string
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("kernel.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	//create vars to hold platform and device id's
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint err_num_devices;
	cl_uint err_num_platforms;
	cl_int err;

	//retrieve available platform - a specific OpenCL implementation
	err = clGetPlatformIDs(1, &platform_id, &err_num_platforms);

	//retrieve available GPU - ie find availablility of physical GPU hardware
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &err_num_devices);

	//create opencl context - platform with a set of available devices
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);

	//create command queue - queue of opencl kernels to run
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, NULL, &err);

	//create opencl buffer object - which is used to setup buffers in GPU memory, transfer memory across from RAM and retrieve the data back. Buffer set to READ_WRITE for the kernel as read & write operations are needed
	cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(int), NULL, &err);

	//setup cl mem object A to be written to
	err = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(int), A, 0, NULL, NULL);

	//create the opencl program using the "kernel.cl" source
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &err);

	//build the program
	err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	//create a kernel from the program. The kernel stores the function to be run and arguments
	cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
	if (err != CL_SUCCESS)
	{	
		//check for build errors
		printf("build error");
		return -1;
	}
	//specify argument 0 for the kernel
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
	
	//setup work items

	//Local work items - describes the number of work-items that make up a work-group
	size_t local_item_size = 64;

	//Global work items - describes the number of global work-items in work_dim dimensions that will execute the kernel 
	size_t global_item_size = LIST_SIZE/2;

	//load local item size from args
	if(argc > 2) {
		local_item_size = atoi(argv[2]);
	}

	//call the kernel function LIST_SIZE + 1 times on the same buffer
	i = 0;
	for (i; i < LIST_SIZE + 1; i++) {

		//set the postion 1 argument - the order value, is used in the kernel
		err = clSetKernelArg(kernel, 1, sizeof(order), (void*)&order);

		//enqueue the kernel to be run on the GPU which specified arguments
		err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

		//check for errors
		if (err != CL_SUCCESS)
		{
			printf("error clEnqueue: %i\n", err);
			return -1;
		}

		//flip order
		if (order == 0) {
			order = 1;
		}
		else {
			order = 0;
		}

	}

	
	//retrieve buffer A from GPU
	err = clEnqueueReadBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(int), A, 0, NULL, NULL);


	//PRINT RESULT
	//for (j = 0; j < LIST_SIZE; j++)
	//	printf("%d ", A[j]);


	//stop timer
	timer = clock() - timer;
	printf("GPU RUNTIME (ms): %li\n", timer * 1000 / CLOCKS_PER_SEC);

	//free memory objects including opencl GPU buffer and other opencl types
	err = clFlush(command_queue);
	err = clFinish(command_queue);
	err = clReleaseKernel(kernel);
	err = clReleaseProgram(program);
	err = clReleaseMemObject(a_mem_obj);
	err = clReleaseCommandQueue(command_queue);
	err = clReleaseContext(context);
	free(A);
	return 0;
}
