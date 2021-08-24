// SIT315 Task 2.1 Part 1 - Matrix Multiplication

#include <iostream> 
#include <fstream>  // Read and Write to files
#include <stdio.h>  // input_output functionality
#include <chrono>   // Timer function
#include <ctime>    // Used to seed random number via srand()

using namespace std;

// decalre global variables
const int Size = 3; // set matrix size
static int matrixA[Size][Size]; // declare matrix A,B & C
static int matrixB[Size][Size];
static long int matrixC[Size][Size];
double elapsed = 0; // set elasped variable to zero

// Function to insert random numbers between 1 - 50 into matrix
void randMatrix(int matrix[Size][Size], int size)
{
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
        {
            matrix[i][j] = rand() % 50 + 1; // Fill with random number 1 to 50
        }
}

// Function to multiple matrix A & B with the result as matrix C
void matrix_multiply(int matrixA[Size][Size], int matrixB[Size][Size],
                     long int matrixC[Size][Size])
{
    for (int i = 0; i < Size; i++)
    {
        for (int j = 0; j < Size; j++)
        {
            matrixC[i][j] = 0;
            for (int k = 0; k < Size; k++)
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
        }
    }
}

// Function to print matrix A,B & C to screen for testing purposes
void print_screen(int matrixA[Size][Size], int matrixB[Size][Size],
                  long int matrixC[Size][Size])
{
    // print to screen - Matrix A
    for (int x = 0; x < Size; x++) // loop to Size of Matrix
    {
        for (int y = 0; y < Size; y++)
        {
            cout << " ";
            cout << matrixA[x][y];
        }
        cout << endl; // when the inner loop is done, go to a new line
    }
    // print to screen - Matrix B
    for (int x = 0; x < Size; x++) // loop to Size of Matrix
    {
        for (int y = 0; y < Size; y++)
        {
            cout << " ";
            cout << matrixB[x][y];
        }
        cout << endl; // when the inner loop is done, go to a new line
    }
    // print to screen - Matrix C
    for (int x = 0; x < Size; x++) // loop to Size of Matrix
    {
        for (int y = 0; y < Size; y++)
        {
            cout << " ";
            cout << matrixC[x][y];
        }
        cout << endl; // when the inner loop is done, go to a new line
    }
}

// Main Program
int main()
{
    srand(time(0));             // seed rand() function using clock time
    randMatrix(matrixA, Size);  // Generate random numbers in matrix A
    randMatrix(matrixB, Size);  // Generate random numbers in matrix B
    auto t1 = chrono::high_resolution_clock::now();     // set t1 at time now (start time)
    matrix_multiply(matrixA, matrixB, matrixC);         // call multiple matrix function and pass in MatA and MatB
    auto t2 = chrono::high_resolution_clock::now();     // t2 = end time
    chrono::duration<double, milli> elapsed = t2 - t1;  // Calculate elapsed time
    //print_screen(matrixA,matrixB, matrixC);           // Enable for testing purposes ONLY Comment out
    cout << endl;
    cout << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // print elapsed time to screen
    cout << endl;

    ofstream resultsFile;           // write results to file
    resultsFile.open("results_SMM.txt");// open file & write elapsed time as Matrix C output to file
    resultsFile << "Matrix Multiplication Execution time :" << elapsed.count() << endl; // elapsed time output to file
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