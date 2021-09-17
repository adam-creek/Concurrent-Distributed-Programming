#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
    int numtasks, dest, source, rank, name_len, tag =1; 
    char name[MPI_MAX_PROCESSOR_NAME];
	char hello[13];
    
    MPI_Status status;

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Find the processor name
    MPI_Get_processor_name(name, &name_len);

    // if the rank is zero, create the message and send to worker noder
	if (rank==0)
	{
		strcpy(hello,"Hello World!");
		dest = 1;

		MPI_Send((void*)hello, 13, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    }
    // if the rank is the worker node, receive the message and output to the console
	else if (rank!=0)
	{
        source = 0;
        
		MPI_Recv((void*)hello, 13, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);

        printf("Activity 1: \n\n");

        printf("Rank %d: Message: %s\n\n",rank,hello);
	}

    // put program to sleep for half a second to ensure output comes after
    // first activity
    usleep(500000);

    // if rank is zero, create the message
     if (rank==0)
    {
        printf("Activity 2: \n\n");
        strcpy(hello, "Hello World!");
    }
    // broadcast the message to all nodes
    MPI_Bcast((void*)hello, 13, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    // print the message, along with the process rank, from each node
    printf("From Rank %d, Message %s\n", rank, hello);
    

    // Finalize the MPI environment
    MPI_Finalize();
}