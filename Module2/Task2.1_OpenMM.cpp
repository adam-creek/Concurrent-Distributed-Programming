// SIT315 Task 2.1 Part 3 - Matrix Multiplication - OMP

#include <iostream> 
#include <fstream>  // Read and Write to files
#include <stdio.h>  // input_output functionality
#include <chrono>   // Timer function
#include <ctime>    // Used to seed random number via srand()
#include <omp.h>    

using namespace std;

// decalre global variables
#define omp_set_num_threads = 4;

const int Size = 250;    // set matrix size
static int matrixA[Size][Size]; // declare matrix A,B & C
static int matrixB[Size][Size];
static long int matrixC[Size][Size];
double elapsed = 0; // set elasped variable to zero

// Function to insert random numbers between 1 - 50 into matrix using OpenMP
void randMatrix(int matrix[Size][Size], int size)
{
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; ++j)
            {
                matrix[i][j] = rand() % 50 + 1; // Fill with random number 1 to 50
            }

    }
}

int main()
{
    srand(time(0));             // seed rand() function using clock time
    randMatrix(matrixA, Size);  // Generate random numbers in matrix A
    randMatrix(matrixB, Size);  // Generate random numbers in matrix B

    auto t1 = chrono::high_resolution_clock::now();     // set t1 at time now (start time)

#pragma omp parallel shared (matrixA, matrixB, matrixC, Size) // implement OpenMP procedure
    #pragma omp for
    {
        for(int i = 0; i < Size; i++)
        {
            for (int j = 0; j < Size; j++)
	        {
		        matrixC[i][j] = 0;
                for (int k = 0; k < Size; k++)
		        {
			        matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
		        }
	        }
        }
    }

    auto t2 = chrono::high_resolution_clock::now();     // t2 = end time for multiplication task
    chrono::duration<double, milli> elapsed = t2 - t1;  // Calculate elapsed time
    cout << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // output elapsed time to screen
    cout << endl;

    ofstream resultsFile;           // write results to file
    resultsFile.open("results_OpenMP_MM.txt");// open file & write elapsed time as Matrix C output to file
    resultsFile << "Matrix Multiplication Execution time :" << elapsed.count() << endl; // elapsed time to putput file
    resultsFile << "Matrix A Output: " << endl; // Matrix results output to file to validate results
    for (int i = 0; i < Size; ++i)
        for (int j = 0; j < Size; ++j)
        {
            resultsFile << " " << matrixA[i][j];
            if (j == Size - 1)
                resultsFile << endl;
        }
    resultsFile << "Matrix B Output: " << endl;
    for (int i = 0; i < Size; ++i)
        for (int j = 0; j < Size; ++j)
        {
            resultsFile << " " << matrixB[i][j];
            if (j == Size - 1)
                resultsFile << endl;
        }
    resultsFile << "Matrix C Output: " << endl;
    for (int i = 0; i < Size; ++i)
        for (int j = 0; j < Size; ++j)
        {
            resultsFile << " " << matrixC[i][j];
            if (j == Size - 1)
                resultsFile << endl;
        }
    resultsFile.close();    // Close file
    return 0;
}