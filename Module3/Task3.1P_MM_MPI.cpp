// Task 3.1 Matrix Multiplication - MPI ONLY (based on lecture 7 example)

#include <iostream>
#include<stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <mpi.h>
#include <chrono>
#include <ctime>

using namespace std;
using namespace std::chrono;

#define BILLION  1000000000L;
int NUM_THREADS = 1;

int SZ = 4;         // set Matrix Size as 4 (allow for input when running program)
int rand_num = 100; // Random number limit used for randonising matrices
int **A, **B, **C;  // Pointers to Matrix A,B & C

// Functions 
void init(int** &matrix, int rows, int cols, bool initialise);
void print( int** matrix, int rows, int cols);
void* add(void* block_id);
void* multiply(void* args);
void head(int num_processes);
void node(int process_rank, int num_processes);


// Main program
int main(int argc, char** argv) {
    if(argc > 1) SZ = atoi(argv[1]);    // pass in argument for size dynamically
	
	srand(time(0));             // seed rand() function using clock time

    MPI_Init(NULL, NULL);

    int num_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    if (SZ%num_processes!=0)  // if # processes does not divide into Matrix Size then determinate program
    {
        if (process_rank == 0) // check if master - print msg once
        printf("Matrix Size not divisible by the number of processes\n");
        MPI_Finalize();
        exit(1);
    }

    if(process_rank == 0) // call head function if rank = 0 
        head(num_processes);
    else
        node(process_rank, num_processes); // otherwise call node function 
    
    MPI_Finalize(); // determinate MPI environment
}

void head(int num_processes)
{
	// set varaibles used in MPI passing of data and main matrix multiplication calculation
    int num_elements_to_scatter_or_gather = (SZ * SZ) / num_processes;
    int num_rows_per_process_from_A = SZ / num_processes;
    int num_elements_to_bcast = (SZ * SZ);
    
    init(A, SZ, SZ, true), init(B, SZ, SZ, true), init(C, SZ, SZ, false);

    print(A, SZ, SZ); // Print Matrix A
    print(B, SZ, SZ); // Print Matrix B

    auto t1 = chrono::high_resolution_clock::now();	// start timer

    // Scatter Matrix A between Head and Nodes and Broadcast Matrix B to all nodes as whole

    MPI_Scatter(&A[0][0], num_elements_to_scatter_or_gather ,  MPI_INT , &A , 0, MPI_INT, 0 , MPI_COMM_WORLD);
    MPI_Bcast(&B[0][0], num_elements_to_bcast , MPI_INT , 0 , MPI_COMM_WORLD);

    //Calculate the matrix C based on number of rows from Matrix A sent to Head

    int localSum;   // Set local sum varaible

    for(int i = 0; i < num_rows_per_process_from_A ; i++) {    // 3 nested loops to iterate over matrix indices 
        for(int j = 0; j < SZ; j++) 
        {
            localSum = 0;
            for (int k = 0; k < SZ; k++)
			{
				localSum += A[i][k] * B[k][j];		// perform matrix multiplication and assign to localSum
			}
            C[i][j] = localSum;						// Assign multiplcation result to matrix C position
        }
    }
    MPI_Gather(MPI_IN_PLACE, num_elements_to_scatter_or_gather , MPI_INT, &C[0][0] , num_elements_to_scatter_or_gather , MPI_INT, 0 , MPI_COMM_WORLD);
    //send the results back to the head node for merging and printing

    auto t2 = chrono::high_resolution_clock::now();	// stop timer

    print(C, SZ, SZ); // Print C Matrix (result)

    chrono::duration<double, milli> elapsed = t2 - t1; // calculate elapsed time

    cout << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // print elapsed time to screen
    cout << endl;
}

void node(int process_rank, int num_processes)
{
	// set varaibles used in MPI passing of data and main matrix multiplication calculation
    int num_rows_per_process_from_A = SZ / num_processes;
    int num_elements_to_bcast = (SZ * SZ);
    int num_elements_to_scatter_or_gather = (SZ * SZ) / num_processes;

    //receive rows of matrix A, and all B 
    init(A, num_rows_per_process_from_A , SZ, false), init(B, SZ, SZ, false), init(C, num_rows_per_process_from_A, SZ, false);

    MPI_Scatter(NULL, num_elements_to_scatter_or_gather , MPI_INT , &A[0][0], num_elements_to_scatter_or_gather, MPI_INT, 0 , MPI_COMM_WORLD);
    MPI_Bcast(&B[0][0], num_elements_to_bcast , MPI_INT , 0 , MPI_COMM_WORLD);
    
    //calculate the assigned rows of matrix C

    int localSum;	// Set local sum varaible

    for(int i = 0; i < num_rows_per_process_from_A ; i++) {       
        for(int j = 0; j < SZ; j++)
        {
            localSum = 0;
            for (int k = 0; k < SZ; k++)
			{
                localSum += A[i][k] * B[k][j]; 	// perform matrix multiplication and assign to localSum
            }
            C[i][j] = localSum;					// Assign multiplcation result to matrix C position
        }
    }

    MPI_Gather(&C[0][0], num_elements_to_scatter_or_gather , MPI_INT, NULL, num_elements_to_scatter_or_gather , MPI_INT, 0 , MPI_COMM_WORLD);
}

void init(int** &A, int rows, int cols, bool initialise) {
    A = (int **) malloc(sizeof(int*) * rows * cols);  		// number of rows * size of int* address in the memory
    int* tmp = (int *) malloc(sizeof(int) * cols * rows); 

    for(int i = 0 ; i < SZ ; i++) {
        A[i] = &tmp[i * cols];
    }
  
    if(!initialise) return;

    for(long i = 0 ; i < rows; i++) {
        for(long j = 0 ; j < cols; j++) {
            A[i][j] = rand() % rand_num; 	// any number less than 100
        }
    }
}

void print( int** A, int rows, int cols) {
  for(long i = 0 ; i < rows; i++) { 		//iterate < rows
        for(long j = 0 ; j < cols; j++) {  	//iterate < cols
            printf("%d ",  A[i][j]); 		// print the cell value
        }
        printf("\n"); 						//at the end of the row, print a new line
    }
    printf("----------------------------\n");
}
