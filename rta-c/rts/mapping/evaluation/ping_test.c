#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define MPI_TAG 10

int main(int argc, char **argv){
	int pid;
	int buffer = 0;
	MPI_Status status;

	MPI_Init(0,0);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	if(pid == 0)
	{
		double start_time, stop_time;
		/* sync peers before ping test (barrier) */
		MPI_Send(&buffer, 1, MPI_REAL, 1, MPI_TAG, MPI_COMM_WORLD);
		MPI_Recv(&buffer, 1, MPI_REAL, 1, MPI_TAG, MPI_COMM_WORLD, &status);
		/* perform ping-test */
		start_time = MPI_Wtime();
		MPI_Send(&buffer, 1, MPI_REAL, 1, MPI_TAG, MPI_COMM_WORLD);
		MPI_Recv(&buffer, 1, MPI_REAL, 1, MPI_TAG, MPI_COMM_WORLD, &status);
		stop_time = MPI_Wtime();
		printf("result in ms: %f\n", (stop_time - start_time) * 1000 );
	} else if(pid == 1)
	{
		/* sync peers before ping test (barrier)*/
		MPI_Recv(&buffer, 1, MPI_REAL, 0, MPI_TAG, MPI_COMM_WORLD, &status);
		MPI_Send(&buffer, 1, MPI_REAL, 0, MPI_TAG, MPI_COMM_WORLD);
		MPI_Recv(&buffer, 1, MPI_REAL, 0, MPI_TAG, MPI_COMM_WORLD, &status);
		sleep(1);
		MPI_Send(&buffer, 1, MPI_REAL, 0, MPI_TAG, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}

