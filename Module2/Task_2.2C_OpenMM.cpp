// Task 2.2C - QuickSort - OpenMP

// c++ Libraries
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <chrono> 
#include <omp.h>

#define omp_set_num_threads = 8;    // set the number of threads used
#define N 10000 // define size for array

using namespace std;
using namespace std::chrono;

double elapsed = 0; // set elasped variable to zero

void populateArray(int arr[N])  // function to populate array with random number between 1 and 100
{
    #pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        arr[i] = rand() % 100 + 1;  
    }
}

void swap(int arr[N], int a, int b) // function to swap values in array position
{
    int tmp = arr[b];
    arr[b] = arr[a];
    arr[a] = tmp;
}

int partition(int arr[N], int low, int high)
{
    int pivot = arr[high]; // set the pivot as last element in array
    int smallest = low - 1;

    for (int i = low; i < high; i++)
    {
        if (arr[i] <= pivot) // if the current element is smaller or equal to than the pivot
        {                    // then increment smallest num & call swap function
            smallest++;
            swap(arr, smallest, i);
        }
    }
    swap(arr, smallest + 1, high);
    return smallest + 1;
}

void quicksort(int arr[N], int low, int high)   // quicksort function
{
#pragma omp parallel
    if (low < high) // check position
    {
        int p = partition(arr, low, high); // set pivot - partition the array
        #pragma omp task
        {
            quicksort(arr, low, p - 1);        // recurive quick sort left partition   
        }
        #pragma omp task
        {
            quicksort(arr, p + 1, high);       // recurive quick sort right partition  
        }
    }  
}

void printArray(int arr[N]) // print array for test validation
{
    int i;
    for (i = 0; i < N; i++)
        cout << arr[i] << " ";
}

int main()
{
    srand(time(0)); // set random function
    
    int *arr = new int[N]; // declare an array with size N
    
    populateArray(arr); // calls populate Array function and pass in array

    auto t1 = chrono::high_resolution_clock::now(); // set t1 at time now (start time)

    #pragma omp parallel // parallel execution 
    {
        #pragma omp single nowait
        {
            quicksort(arr, 0, N-1);      
        }
    } 

    // calls quicksort function and passes in array,
    // 0 as starting index and size -1 as end index

    auto t2 = chrono::high_resolution_clock::now();    // t2 = end time for multiplication task
    chrono::duration<double, milli> elapsed = t2 - t1; // Calculate elapsed time

    cout << endl
         << "Elapsed Time for Process: " << elapsed.count() << " Milli-Seconds"; // output elapsed time to screen
    cout << endl;

    return 0;
}