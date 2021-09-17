// Seminar 7 - Activity 2
#include <iostream>
#include <time.h>
#include <chrono>
#include <mpi.h>
#include <stdio.h>

using namespace std;

void randomVector(int vector[], int size)
{
    for (int i = 0; i < size; i++)
    {
        vector[i] = rand() % 100;
    }
}

int main(int argc, char* argv[]){

    // set the seed
    srand(time(NULL));

    // define variables
    int numtasks, dest, source, rank, name_len, tag =1;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks); 

    // Get the processor rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    unsigned long size = 10000000;

    int elements_per_process = size/numtasks;

    // define the memory allocation for the complete vector and the
    // vector subsets that each process will deal with
    int* v1 = (int* ) malloc(size * sizeof(int *));
    int* v2 = (int *) malloc(size * sizeof(int *)); 
    int* v3 = (int *) malloc(size * sizeof(int *)); 
    int* v1_subset = (int* ) malloc(elements_per_process * sizeof(int *));
    int* v2_subset = (int* ) malloc(elements_per_process * sizeof(int *));   
    int* v3_subset = (int *) malloc(elements_per_process * sizeof(int *));    

    // define the timer variables
    double start, stop, duration;

    // initialize the timer start on the root process
    if (rank == 0){
        start = MPI_Wtime(); 
    }

    // scatter the first vector amongst the processes
    MPI_Scatter(v1, elements_per_process, MPI_INT, v1_subset, elements_per_process, MPI_INT,
                0, MPI_COMM_WORLD);

    // scatter the second vector amongst the processes
    MPI_Scatter(v2, elements_per_process, MPI_INT, v2_subset, elements_per_process,MPI_INT,
                0, MPI_COMM_WORLD); 

    // call the randomVector function on the subsets of each vector
    randomVector(v1_subset, elements_per_process);
    randomVector(v2_subset, elements_per_process);


    // define the local variables local and total Sum
    long localSum = 0;
    long totalSum = 0;

    // loop through each vector subset and add the elements at the same index in vectors 1 and 2
    // to generate the corresponding element in vector 3
    for (int i = 0; i < elements_per_process; i++){
        v3_subset[i] = v1_subset[i] + v2_subset[i];
        // add the element in vector three to local variable localSum
        localSum += v3_subset[i];
    }

    // gather the subsets of vector 3 to the root process to form the whole vector
    MPI_Gather(v3_subset, elements_per_process, MPI_INT, v3, elements_per_process, MPI_INT, 0,
        MPI_COMM_WORLD);

    // use the MPI reduce function to add the two totals of each of the subsets and output
    // the totalSum variable
    MPI_Reduce(&localSum, &totalSum, 1, MPI_INT,
               MPI_SUM, 0, MPI_COMM_WORLD);

    // initialise the stop variable on the root process
    if (rank == 0){
        stop = MPI_Wtime();
        // calculate the duration variable
        duration =  (stop - start)*1000000;
        cout << "Time taken by function: "
        << duration << " microseconds" << "\n";

        cout << "Total sum of elements in vector 3: " << totalSum << "\n";
    }
    
    return 0;
}