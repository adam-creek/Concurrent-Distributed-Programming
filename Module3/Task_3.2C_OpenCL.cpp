//Task 3.2C - QuickSort - MPI + OpenCL
// Reference: CL Lecture Example
// To build: mpicxx Task_3.2C_OpenCL.cpp -lOpenCL
// To execute: mpirun ./a.out OR mpirun -np 2 ./a.out (head and node)
// Reference: https://www.codeproject.com/articles/1082879/parallel-implementation-and-evaluation-of-quicksor


// Kernel program (uncomment and place in CL file to run)

/* __kernel void Quicksort(__global int *array, __global int *stack)

{
    const int chunk_size = get_global_id(0);

    int left = 0; // start index
    int right = chunk_size; // end index
    int top = -1; // top of stack
    int temp; // temp variable for swaping

    stack[++top] = left; // left pair  - > stack
    stack[++top] = right; // right pair  - > stack

    while (top >= 0) // Iterate until stack is empty
    {
        //Get subarray using the first left:right pair
        right = stack[top--]; 
        left = stack[top--];

        int pivot = array[right]; // set pivot as right element
        int idx = left; // element < pivot movr to left of index

        for (int i = left; i < right; i++) // iterate whilst < right, if element < = pivot move element before index
        {
            if (array[i] <= pivot)
            {
                temp = array[i];
                array[i] = array[idx];
                array[idx] = temp;
                idx++;
            }
        }
      
        // Swap procedure
        temp = array[idx];
        array[idx] = array[right];
        array[right] = temp;

        if (idx - 1 > left) // element index < pivot move to stack 
        {
            stack[++top] = left;
            stack[++top] = idx - 1;
        }

        if (idx + 1 < right) // element index > pivot move to stack 
        {
            stack[++top] = idx + 1;
            stack[++top] = right;
        }
    }
} */

// C++ Program

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <mpi.h>

using namespace std;
using namespace std::chrono;

// decalre variables
int SZ = 10;
int *data;          // declare data pointer 
int *dataStack;    // declare data stack pointer
int *localArray;	// declare local array pointer

//Declare the variables needed for OpenCL outside main
cl_mem buf_data;
cl_mem buf_stack;
cl_device_id device_id;
cl_context context;
cl_program program;
cl_kernel kernel;
cl_command_queue queue;
cl_event event = NULL;

int err;

//Declare all the functions for OpenCL
cl_device_id create_device();
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);
void setup_kernel_memory();
void copy_kernel_args();
void free_memory();

//Declare functions
void init(int *&array, int *&stack, int size);
void printArray(int *array, int size);
void head(int num_processes);
void node (int process_rank, int num_processes);

// Main Program
int main(int argc, char **argv)
{
    if (argc > 1)   
    {
        SZ = atoi(argv[1]); // Allow to pass in size argument (0 - 99)
    }
	
	srand(time(0));  // set random seed
	
    MPI_Init(NULL, NULL); //Initializing MPI environment

    int num_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes); // determine number of processes

    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank); // determine rank of processes
	
	init(data, dataStack, SZ); // initialse array and stack with data and dataStack
	
	if(process_rank == 0) // call head function if rank = 0, otherwise node
		head(num_processes);
    else
        node(process_rank, num_processes);
	
	MPI_Finalize();//Close MPI environment
}

void init(int *&array, int *&stack, int size) // initialse array and stack
{
    array = (int *)malloc(sizeof(int) * size);
    stack = (int *)malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++)
    {
        array[i] = rand() % 100; // Random number less than 100 ( - )
    }
}

void printArray(int *array, int size) // print array for test validation
{  
    for (int i=0; i < size; i++) 
	{
        cout<<array[i]<<" "; 
	}
	printf("\n----------------------------\n");
} 

void head(int num_processes) // Head function
{
	int chunk_size = SZ / num_processes;	// chunk size for each process
	
	localArray = (int *)malloc(chunk_size * sizeof(int *));	// allocate memory to local array which process will calculate based on chunk size
	
	printf("Unsorted Array:\n"); // print unsorted Array
	printArray(data, SZ);

	auto t1 = chrono::high_resolution_clock::now(); // set timer
	
	MPI_Scatter(data, chunk_size,  MPI_INT , localArray, chunk_size, MPI_INT, 0 , MPI_COMM_WORLD);	// scattter chucks to each process from head
    
	size_t global[1] = {(size_t)SZ};

    setup_openCL_device_context_queue_kernel((char *)"./QuicksortCL.cl", (char *)"Quicksort");
    setup_kernel_memory();
    copy_kernel_args();
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);
    clEnqueueReadBuffer(queue, buf_data, CL_TRUE, 0, SZ *  sizeof(int), &data[0], 0, NULL, NULL);

    //Gather the different MPI processes back together after the OpenCL code
    MPI_Gather(MPI_IN_PLACE, chunk_size , MPI_INT, localArray, chunk_size, MPI_INT, 0 , MPI_COMM_WORLD);
	
    printf("Sorted Array:\n"); // print sortedArray
	printArray(data,SZ);

    auto t2 = chrono::high_resolution_clock::now();	// stop timer

    chrono::duration<double, milli> elapsed = t2 - t1; // calculate elapsed time
    cout << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // print elapsed time to screen
    cout << endl;
	
	free_memory();    // Call Free memory function
}

void node (int process_rank, int num_processes)
{
	int chunk_size = SZ / num_processes; // chunk size for each process
	
	localArray = (int *)malloc(chunk_size * sizeof(int *));	// allocate memory to local array which process will calculate based on chunk size
	
	MPI_Scatter(NULL, chunk_size,  MPI_INT , localArray, chunk_size, MPI_INT, 0 , MPI_COMM_WORLD);	// scattter chucks to each process

    size_t global[1] = {(size_t)SZ};

    setup_openCL_device_context_queue_kernel((char *)"./QuicksortCL.cl", (char *)"Quicksort");
    setup_kernel_memory();
    copy_kernel_args();
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);
    clEnqueueReadBuffer(queue, buf_data, CL_TRUE, 0, SZ * chunk_size * sizeof(int), &data[0], 0, NULL, NULL);

    //Gather the different MPI processes back together after the OpenCL code
    MPI_Gather(localArray, chunk_size , MPI_INT, NULL, chunk_size, MPI_INT, 0 , MPI_COMM_WORLD);
	
	free_memory();    // Call Free memory function
}

void free_memory() // free memory from objects, variables etc
{
    clReleaseMemObject(buf_data);
    clReleaseMemObject(buf_stack);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(data);
    free(dataStack);
}

void copy_kernel_args() // set kernel arguments
{
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buf_data);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&buf_stack);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory() // set kernel memory, creates buffer object
{
    buf_data = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);
    buf_stack = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);

    clEnqueueWriteBuffer(queue, buf_data, CL_TRUE, 0, SZ * sizeof(int), &data[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, buf_stack, CL_TRUE, 0, SZ * sizeof(int), &dataStack[0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname) // create context for new device, queue, build program
{
    device_id = create_device();
    cl_int err;

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0)
    {
        perror("Couldn't create a command queue");
        exit(1);
    };

    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    };
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }
    return program;
}

cl_device_id create_device()
{
    cl_platform_id platform;
    cl_device_id dev;
    int err;

    err = clGetPlatformIDs(1, &platform, NULL);
    if (err < 0)
    {
        perror("Couldn't identify a platform");
        exit(1);
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    }
    if (err < 0)
    {
        perror("Couldn't access any devices");
        exit(1);
    }
    return dev;
}