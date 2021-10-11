// Task 3.2C - QuickSort - MPI
// Reference: Seminar Task Wk 7
// To build: mpicxx Task_3.2C_QS_MPI.cpp
// To execute: mpirun ./a.out OR mpirun -np 2 ./a.out (head and node)
// Reference: https://www.codeproject.com/articles/1082879/parallel-implementation-and-evaluation-of-quicksor
// Reference: https://www.geeksforgeeks.org/implementation-of-quick-sort-using-mpi-omp-and-posix-thread/
// Quicksort iteratively

// c++ Libraries
#include <cstdlib>
#include <iostream>
#include <cstdlib>
#include <ctime> 
#include <chrono>
#include <mpi.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stack>

using namespace std;
using namespace std::chrono;

long SZ = 10; // size of array

void populateArray(int *array, int size); // function to populate array with random number between 1 and 100
void printArray(int *array, int size);
void swap(int *a, int *b);
void quicksort(int *array, int *stack, int chunk_size); // perform iterative QuickSort
int *merge(int *chunk1, int chunk1_size, int *chunk2, int chunk2_size); // Merge Arrays

// Main Program
int main(int argc, char **argv)
{
    srand(time(0));   // set random function

    int *data = NULL; // pointer to data 
    int *stack = NULL; // pointer to stack
    int *chunk; // ponter to chunk
    int *chunk_stack; // pointer to chunk stack

    MPI_Status status;

    auto t1 = chrono::high_resolution_clock::now();     // set t1 at time now (start time)
    
    MPI_Init(NULL, NULL); // Initialise MPI environment

    int num_processes; 
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes); //Get number of processes

    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank); // Get the rank of each of the processes
    
    if (SZ%num_processes!=0)  // if # processes does not divide into size of Array then determinate program
    {
        if (process_rank == 0) // check if master - print msg once
        printf("Array Size not divisible by the number of processes\n");
        MPI_Finalize();
        exit(1);
    }
    
    if (process_rank == 0) // Execute if master
    {
        data = (int *)malloc(SZ * sizeof(int)); // Allocate memory
        stack = (int *)malloc(SZ * sizeof(int)); // Allocate memory

        populateArray(data, SZ); // call function to populate array
        printf("Unsorted array: \n");
        printArray(data, SZ);   // call function to Print array
    }

    MPI_Barrier(MPI_COMM_WORLD); // Block all processes

    MPI_Bcast(&SZ, 1, MPI_INT, 0, MPI_COMM_WORLD); // Broadcast Size to all nodes

    int chunk_size = SZ / num_processes; // Determine number of elements in each chunk

    chunk = (int *)malloc(chunk_size * sizeof(int)); // allocate memory 
    chunk_stack = (int *)malloc(chunk_size * sizeof(int)); // Allocate memory

    MPI_Scatter(data, chunk_size, MPI_INT, chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD); // Scatter data to nodes
    MPI_Scatter(stack, chunk_size, MPI_INT, chunk_stack, chunk_size, MPI_INT, 0, MPI_COMM_WORLD); // scatter stack to nodes

    int local_chunk_size = chunk_size; // Initialise local_chunk_size

    quicksort(chunk, chunk_stack, chunk_size); // Call Quicksort function

    for (int step = 1; step < num_processes; step = 2 * step)//  Each step merges two chunks until one remains
    {
        // Detemine which node is sending, once send process is completed.
        if (process_rank % (2 * step) != 0)
        {
            MPI_Send(chunk, local_chunk_size, MPI_INT, process_rank - step, 0, MPI_COMM_WORLD);
            break;
        }

        // Determine which process is receiving.
        if (process_rank + step < num_processes)
        {
            int chunk_merge_SZ; // initialise variable to store new chunk

            if (SZ >= chunk_size * (process_rank + 2 * step)) // compare chunk size to array size
            {
                chunk_merge_SZ = chunk_size * step;
            }
            else // Final Merge
            {
                chunk_merge_SZ = SZ - chunk_size * (process_rank + step);
            }

            int *chunk_merge; // Allocate memory to accomodate chunk received
            chunk_merge = (int *)malloc(chunk_merge_SZ * sizeof(int));

            MPI_Recv(chunk_merge, chunk_merge_SZ, MPI_INT, process_rank + step, 0, MPI_COMM_WORLD, &status); // Receive chunk from merged process

            data = merge(chunk, local_chunk_size, chunk_merge, chunk_merge_SZ); // merged chunks

            chunk = data; // Assign merged data to chunk
            local_chunk_size = local_chunk_size + chunk_merge_SZ; // increase local chunk to accomodate for merged chunks
        }
    }

    if (process_rank == 0)
    {
        printf("Sorted Array: \n");
        printArray(chunk, SZ);   // call function to Print array
        
        auto t2 = chrono::high_resolution_clock::now();     // t2 = end time for multiplication task
        chrono::duration<double, milli> elapsed = t2 - t1;  // Calculate elapsed time
    
        cout << endl << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // output elapsed time to screen
        cout << endl;
        }
    
    MPI_Finalize();
    return 0;
}

void populateArray(int *array, int size) // function to populate array with random number between 1 and 100
{ 
    for (int i=0; i< size; i++) 
    {
        array[i] = rand() % 100 + 1;  
    }
}

void quicksort(int *array, int *stack, int chunk_size)
{
    int left = 0; // start idx
    int right = chunk_size - 1; // end idx

    int top = -1; // Initialise Top of stack

    // Left: Right pair to push into stack
    stack[++top] = left;
    stack[++top] = right;

    while (top >= 0)  // loop while empty
    {
        // First left:right pair from the stack (boundary for sub array)
        right = stack[top--];
        left = stack[top--];

        int pivot = array[right]; // Take right element as pivot (could be random or left)

        int idx = left; // Numbers < pivot = left, other right

        for (int i = left; i < right; i++) // If number <= pivot, swap number and idx, increment idx
        {
            if (array[i] <= pivot)
            {
                swap(&array[i], &array[idx]); // Call swap function
                idx++; // increment index
            }
        }
        swap(&array[idx], &array[right]); // Call swap function to swap index and pivot

        if (idx - 1 > left) // idx < pivot are added to stack
        {
            stack[++top] = left;
            stack[++top] = idx - 1;
        }

        if (idx + 1 < right) // idx > pivot are added to stack
        {
            stack[++top] = idx + 1;
            stack[++top] = right;
        }
    }
}

int *merge(int *chunk1, int chunk1_size, int *chunk2, int chunk2_size)
{
    // Allocate memory for merged chunk
    int *merged_chunk = (int *)malloc((chunk1_size + chunk2_size) * sizeof(int));

    int i = 0; // initialise varaible for index of chunk
    int j = 0; // initialise varaible for index of chunk

    //Iterate through each chunk indices to merge together
    for (int k = 0; k < chunk1_size + chunk2_size; k++)
    {
        // Check Chunk 1 if no elements, add elements from chunk 2 to merged chunk
        if (i >= chunk1_size)
        {
            merged_chunk[k] = chunk2[j];
            j++;
        }
        // Check Chunk 2 if no elements, add elements from chunk 1 to merged chunk
        else if (j >= chunk2_size)
        {
            merged_chunk[k] = chunk1[i];
            i++;
        }
        // Compare elements in Chunk 1 & 2, if 1 < 2, place element from chunk 1 in merged chunk
        else if (chunk1[i] < chunk2[j])
        {
            merged_chunk[k] = chunk1[i];
            i++;
        }
        // Otherwise chunk 2 element is smallest and add to merged chunk
        else
        {
            merged_chunk[k] = chunk2[j];
            j++;
        }
    }
    return merged_chunk;
}

void printArray(int *array, int size) // print array for test validation
{ 
    if (size < 20)
    {
      for (int i=0; i < size; i++) 
      {
         printf("%d ", array[i]);   
      }
        printf("\n");
    }
    else
    {
        for (int i = 0; i < 5; i++) // print first 5
        {
            printf("%d  ", array[i]);
        }
        printf("....  ");

        for (int i = size - 5; i < SZ; i++) // print end 5
        {
            printf("%d  ", array[i]);
        }
        printf("\n");
    }
} 

void swap(int *a, int *b) // function to swap values in array position
{
    int tmp = *b; 
    *b = *a;
    *a = tmp;
}