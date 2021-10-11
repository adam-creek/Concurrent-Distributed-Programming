//Task 3.1P - Matrix Multi - MPI + OpenCL (based on lecture examples)

// CL file (kernel)
__kernel void matrix_multiply(const int SZ, const __global int* A, const __global int* B, __global int* C)
{
    // Reference global ids
    int i = get_global_id(0);
    int j = get_global_id(1);
    int k = get_global_id(2);

    // Reference matrix index
    int A_index = (i * SZ) + k;
    int B_index = (k * SZ) + j;
    int C_index = (i * SZ) + j;

    // calculate matrix multiplication & Sum 
    int localSum = (A[A_index] * B[B_index]);
    C[C_index] += localSum;
}


// C++ .cpp file

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <mpi.h>
#include <chrono>
#include <stdio.h>
#include <CL/cl.h>

using namespace std;
using namespace std::chrono;

//Declare variables
int SZ = 3;
int *A, *B, *C;  

cl_mem bufA, bufB, bufC;
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
void setup_kernel_memory(int num_rows_per_process_from_A); // Pass in number of rows variable not full matrix size
void copy_kernel_args();
void free_memory();

//Declare functions
void init(int* &matrix, int rows, int cols, bool initialise);
void print( int* matrix, int rows, int cols);
void head(int num_rows_per_process_from_A, int broadcast_size,int num_elements_to_scatter_or_gather);
void node(int num_rows_per_process_from_A, int broadcast_size,int num_elements_to_scatter_or_gather);

int main(int argc, char** argv) {

    if (argc > 1)
        SZ = atoi(argv[1]); // Accept Matrix Size as dynamic input

    srand(time(0));  // set random seed

    //Initializing MPI environment
    MPI_Init(NULL, NULL);

    int num_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    // set variables for MPI processes (as opposed setting in head and node)
    int num_rows_per_process_from_A = SZ/ num_processes;
    int broadcast_size = (SZ * SZ);
    int num_elements_to_scatter_or_gather = (SZ * SZ) / num_processes;


    if (SZ%num_processes!=0)  // if # processes does not divide into Matrix Size then determinate program
    {
        if (process_rank == 0) // check if master - print msg once
            printf("Matrix Size not divisible by the number of processes\n");
            MPI_Finalize();
            exit(1);
    }

    if(process_rank == 0)
        // call head if rank = 0
        head(num_rows_per_process_from_A, broadcast_size,num_elements_to_scatter_or_gather);
    else
        // otherwise call node
        node(num_rows_per_process_from_A, broadcast_size, num_elements_to_scatter_or_gather);
    MPI_Finalize();//Close MPI environment
}

void head(int num_rows_per_process_from_A, int broadcast_size,int num_elements_to_scatter_or_gather)
{
    //Initiliaze matrices - full matrix size
    init(A, SZ, SZ, true), init(B, SZ, SZ, true), init(C, SZ, SZ, false);

    print(A,SZ,SZ); // print matrix A
    print(B,SZ,SZ); // print matrix B

    //Start timer for matrix calulation (head only)
    auto t1 = chrono::high_resolution_clock::now();

    //Scatter the data in A across all processes evenly, broadcast the whole of B to all processes
    MPI_Scatter(&A[0], num_elements_to_scatter_or_gather,  MPI_INT , &A , 0, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&B[0], broadcast_size , MPI_INT , 0 , MPI_COMM_WORLD);

    size_t global[3] = {(size_t)num_rows_per_process_from_A, (size_t)SZ, (size_t)SZ};

    setup_openCL_device_context_queue_kernel((char *)"./matrix_kernel.cl", (char *)"matrix_multiply");
    setup_kernel_memory(num_rows_per_process_from_A); // pass in number of rows matrix A to process
    copy_kernel_args();
    clEnqueueNDRangeKernel(queue, kernel, 3, NULL, global, NULL, 0, NULL, &event); // set 3 as number of arguments
    clWaitForEvents(1, &event);
    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, SZ*num_rows_per_process_from_A*sizeof(int), &C[0], 0, NULL, NULL);

    //Gather the different MPI processes back together after the OpenCL code
    MPI_Gather(MPI_IN_PLACE, num_elements_to_scatter_or_gather , MPI_INT, &C[0] , num_elements_to_scatter_or_gather , MPI_INT, 0 , MPI_COMM_WORLD);

    //Stop timer for matrix calulation (head only)
    auto t2 = chrono::high_resolution_clock::now();	// stop timer
    
    print(C,SZ,SZ); // Print matrix C

    // Print duration
    chrono::duration<double, milli> elapsed = t2 - t1; // calculate elapsed time
    cout << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // print elapsed time to screen
    cout << endl;

    free_memory();// Call Free the memory 
}

void node(int num_rows_per_process_from_A, int broadcast_size,int num_elements_to_scatter_or_gather)
{
    init(A,num_rows_per_process_from_A, SZ, false), init(B, SZ, SZ, false), init(C, num_rows_per_process_from_A, SZ, false);

    //Receive Matrix A (portion)
    MPI_Scatter(NULL, num_elements_to_scatter_or_gather , MPI_INT , &A[0], num_elements_to_scatter_or_gather, MPI_INT, 0 , MPI_COMM_WORLD);
    
    // Receive Matrix B (All)
    MPI_Bcast(&B[0], broadcast_size , MPI_INT , 0 , MPI_COMM_WORLD);
    
    size_t global[3] = {(size_t)num_rows_per_process_from_A, (size_t)SZ, (size_t)SZ};

    setup_openCL_device_context_queue_kernel((char *)"./matrix_kernel.cl", (char *)"matrix_multiply");
    setup_kernel_memory(num_rows_per_process_from_A); // pass in number of rows matrix A to process
    copy_kernel_args();
    clEnqueueNDRangeKernel(queue, kernel, 3, NULL, global, NULL, 0, NULL, &event); // set 3 as number of arguments
    clWaitForEvents(1, &event);
    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, SZ*num_rows_per_process_from_A*sizeof(int), &C[0], 0, NULL, NULL);

    // Return portion 
    MPI_Gather(&C[0], num_elements_to_scatter_or_gather , MPI_INT, NULL, num_elements_to_scatter_or_gather , MPI_INT, 0 , MPI_COMM_WORLD);
    
    free_memory(); // Free memory
}

// Function to initialise matrix 
void init(int* &matrix, int rows, int cols, bool initialise) 
{
    matrix = (int *)malloc(sizeof(int*) * rows * cols); 

    for(long i = 0 ; i < rows*cols; i++) 
    {
        matrix[i] = 0; // set matrix to 0
    }

    if(!initialise) return;

    for(long i = 0 ; i < rows * cols; i++) 
    {
        matrix[i] = rand() % 100; // random number between 0 and 100
    }
}

// Print matrix
void print( int* A, int rows, int cols) {
  for(long i = 0 ; i < rows; i++) { 		//iterate < rows
        for(long j = 0 ; j < cols; j++) {  	//iterate < cols
            printf("%d ",  A[i * SZ + j]); 	// print the cell value
        }
        printf("\n"); 						//at the end of the row, print a new line
    }
    printf("----------------------------\n");
}

//OpenCL Functions
void free_memory()
{
    //free the buffers
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);

    //free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    // free matrix
    free(A);
    free(B);
    free(C);
}

void copy_kernel_args()
{
    //sets the kernel arguments for the kernel - SZ and each matrix
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufA);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&bufB);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&bufC);
    
    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory(int num_rows_per_process_from_A)
{
    //clCreateBuffer function creates a buffer object (matrix A & B read only, Matrix C write only)

    bufA = clCreateBuffer(context, CL_MEM_READ_ONLY, SZ * num_rows_per_process_from_A * sizeof(int), NULL, NULL);
    bufB = clCreateBuffer(context, CL_MEM_READ_ONLY, SZ * SZ * sizeof(int), NULL, NULL);
    bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SZ * SZ * sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0, SZ * num_rows_per_process_from_A * sizeof(int), &A[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufB, CL_TRUE, 0, SZ * SZ * sizeof(int), &B[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufC, CL_TRUE, 0, SZ * num_rows_per_process_from_A * sizeof(int), &C[0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device();
    cl_int err;

    //clCreateContext creates context for the new device
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    //Creates a host or device command-queue on a new device
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

    /* Read program file and place content into buffer */
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

    //clCreateProgramWithSource
    program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    // Build program 
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }
    return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // Check if GPU is avaliable 
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // Access CPU if GPU not found
      printf("GPU not found\n");
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}