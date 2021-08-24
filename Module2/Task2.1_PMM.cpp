// SIT315 Task 2.1 Part 2 - Matrix Multiplication - Parallel Algorithm (Revised)

#include <iostream> 
#include <fstream>  // Read and Write to files
#include <stdio.h>  // input_output functionality
#include<stdlib.h> 
#include <chrono>   // Timer function
#include <ctime>    // Used to seed random number via srand()
#include <pthread.h>  // Library for multithreading
#include <thread>
    
using namespace std;

#define MAXTHREADS 8    // vary between 2 and 8 (MAX)

// global variables

unsigned long Size = 3L; // set matrix size
double elapsed = 0; // set elapsed (time variable) to zero

struct MultiTask    // Struct to pass in multiple arguments to matrix_multiply function (start & end for parition)
{
    int ** mA;
    int ** mB;
    int ** mC;
    int start;
    int end;
};

struct RandomTask    // Struct to pass in multiple arguments to randMatrix function
{
    int ** m1; 
    int ** m2;
    int seed;
    int start;
    int end;
};

void* randMatrix(void *args) // Function to populate matrix with random numbers
{
    RandomTask *task =((struct RandomTask *)args);

    srand(time(NULL) * task->seed);     // set random seed
    for(int i = task->start; i < task->end; i++)
        for (int j = 0; j < Size; ++j)
        {
            task->m1[i][j] = rand() % 50 + 1; // Fill matrix A  with random number 1 to 50
            task->m2[i][j] = rand() % 50 + 1; // Fill matrix B with random number 1 to 50
        }
    return 0;
}

// Function to multiple matrix A & B with the result as matrix C using multi-threading
void* matrix_multiply(void* args)
{
    MultiTask *task =((struct MultiTask *)args);

    for(int i = task->start; i < task->end; i++)
    { 
        for (int j = 0; j < Size; j++)
		{
			task->mC[i][j] = 0;
            for (int k = 0; k < Size; k++)
			{
				task->mC[i][j] += task->mA[i][k] * task->mB[k][j];
			}
		}
	}
    return 0;
}

int main()
{
    // unsigned int n = std::thread::hardware_concurrency();   // check MAX # concurrent thread
    // std::cout << n << " concurrent threads are supported.\n";

    int seed = 10;  // set seed for random number
        
    int ** mA = NULL,** mB = NULL,** mC = NULL;  
    
    // allocate memory for 2D array
    mA = (int**) malloc(Size*sizeof(int*));  
    for (int i = 0; i < Size; i++)  
        mA[i] = (int*) malloc(Size*sizeof(int)); 

    mB = (int**) malloc(Size*sizeof(int*));  
    for (int i = 0; i < Size; i++)  
        mB[i] = (int*) malloc(Size*sizeof(int)); 
    
    mC = (int**) malloc(Size*sizeof(int*));  
    for (int i = 0; i < Size; i++)  
        mC[i] = (int*) malloc(Size*sizeof(int)); 

    auto t1 = chrono::high_resolution_clock::now(); // set timer

    pthread_t threads[MAXTHREADS]; // declare number of threads
    pthread_t random_threads[MAXTHREADS]; // Use for threads into random generator

    int partition_size = Size / (MAXTHREADS/2);

    // create and join threads for random fill matrix function
    for(size_t i = 0; i < MAXTHREADS; i++)
    {
        struct RandomTask *task = (struct RandomTask *)malloc(sizeof(struct RandomTask));
        task->seed = seed++;
        task->m1=mA;    // set matrix that needs to be populated 
        task->m2=mB;    // set matrix that needs to be populated
        task->start = i * partition_size; // set start index of loop
        task->end = (i+1) == MAXTHREADS ? Size :((i+1) * partition_size); // calc end index of loop
        pthread_create(&random_threads[i], NULL, randMatrix, (void*)task); // start random populate thread
    }

    for(size_t i = 0; i < MAXTHREADS; i++)  // Join Threads
    {
        pthread_join(threads[i], NULL);
        }

    partition_size = Size / MAXTHREADS;
    // create and join threads for matrix_multiply function
    for(size_t i = 0; i < MAXTHREADS; i++)
    {
        struct MultiTask *task = (struct MultiTask *)malloc(sizeof(struct MultiTask));

        task->mA=mA;    // set matrix that needs to be used in calc
        task->mB=mB;      // set matrix that needs to be used in cal
        task->mC=mC;    // set matrix that needs to be calculated result
        task->start = i * partition_size; // set start index of loop
        task->end = (i+1) == MAXTHREADS ? Size :((i+1) * partition_size); // calc end index of loop
        pthread_create(&threads[i], NULL, matrix_multiply, (void*)task); // start matrix multiply thread
    }

    for(size_t i = 0; i < MAXTHREADS; i++)  // Join Threads
    {
        pthread_join(threads[i], NULL);
    }

    auto t2 = chrono::high_resolution_clock::now();     // t2 = end time for multiplication task

    chrono::duration<double, milli> elapsed = t2 - t1;  // Calculate elapsed time
    cout << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // Time elspaed output to screen
    cout << endl;

    ofstream resultsFile;           // write results to file
    resultsFile.open("results_PMM.txt");// open file & write elapsed time as Matrix C output to file
    resultsFile << "Matrix Multiplication Execution time :" << elapsed.count() << endl; // elasped time print to output file
    resultsFile << "Matrix A Output: " << endl; // Matrix results print to file for testing

    for (int i = 0; i < Size; ++i)
        for (int j = 0; j < Size; ++j)
        {
            resultsFile << " " << mA[i][j];
            if (j == Size - 1)
                resultsFile << endl;
        }

    resultsFile << "Matrix B Output: " << endl;

    for (int i = 0; i < Size; ++i)
        for (int j = 0; j < Size; ++j)
        {
            resultsFile << " " << mB[i][j];
            if (j == Size - 1)
                resultsFile << endl;
        }
    
    resultsFile << "Matrix C Output: " << endl;

    for (int i = 0; i < Size; ++i)
        for (int j = 0; j < Size; ++j)
        {
            resultsFile << " " << mC[i][j];
            if (j == Size - 1)
                resultsFile << endl;
        }
    resultsFile.close();    // Close file
    free(mA); // free memory
    free(mB);
    free(mC);
    return 0;
}