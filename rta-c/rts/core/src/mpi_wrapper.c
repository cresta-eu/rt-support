/*
 * rta-c - runtime system administration component.
 *  Copyright (C) 2014  Michael Schliephake
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MPI_WRAPPER_IMPLEMENTATION
#include "mpi_wrapper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_PROCS 400
#define MAX_CALIPER 30   // We use calipers to measure iterations

#define BYTE_SCALE  (1024*1024)  // MB
#define BYTE_UNIT   "MB"

static int myrank, num_tasks;
static int current_caliper;
static int tracing;


// structure to store metrics
typedef struct p2p_info {
	long int num_calls;
	double   num_bytes;
} P2PCommInfo;


P2PCommInfo MPI_Send_stats[MAX_CALIPER][MAX_PROCS];
P2PCommInfo MPI_Recv_stats[MAX_CALIPER][MAX_PROCS];



// Function to mark the different iterations
// Param caliper can be the iteration number
void PDC_Loop_Init( int caliper_id ) {

   current_caliper = caliper_id;

}


static void traceInit()
{
    // Initializing data structures

    PMPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    PMPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if (num_tasks > MAX_PROCS)
    {
        if (myrank == 0)
            fprintf(stderr,
                    "Too many processes for communication tracing. Exiting...\n");
        __real_MPI_Finalize();
        exit(-1);
    }
    current_caliper = 0;
    memset(MPI_Send_stats, 0, MAX_PROCS*MAX_CALIPER*sizeof(struct p2p_info));
    memset(MPI_Recv_stats, 0, MAX_PROCS*MAX_CALIPER*sizeof(struct p2p_info));

    tracing = 1;
}

extern "C" int __wrap_MPI_Init(int *argc, char ***argv)
{
  int rc;

  rc = __real_MPI_Init(argc, argv);
  traceInit();

  return rc;
}

extern "C" int __wrap_MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    int rc;

    rc = __real_MPI_Init_thread(argc, argv, required, provided);
    traceInit();

    return rc;
}

static void record_comm_data(P2PCommInfo *stats, MPI_Datatype type, int count)
{
    int type_size;

    MPI_Type_size(type, &type_size);

    stats->num_calls++;
    stats->num_bytes += count * type_size;
}

extern "C" int __wrap_MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest,
        int tag, MPI_Comm comm, MPI_Request *request)
{
    int rc = __real_MPI_Isend(buf, count, datatype, dest, tag, comm, request);

    if (tracing)
        record_comm_data(&MPI_Send_stats[current_caliper][dest], datatype, count);

    return rc;
}

extern "C" int __wrap_MPI_Recv(void *buf, int count, MPI_Datatype datatype,
		    int source, int tag,
		    MPI_Comm comm, MPI_Status *status)
{
    int rc = __real_MPI_Recv(buf, count, datatype, source, tag, comm, status);

    if (tracing)
        record_comm_data(&MPI_Recv_stats[current_caliper][source],
			 datatype, count);

    return rc;
}

extern "C" int __wrap_MPI_Send(void *buf, int count, MPI_Datatype type,
		    int dest, int tag, MPI_Comm comm)
{
    int rc = __real_MPI_Send(buf, count, type, dest, tag, comm);

    if (tracing)
        record_comm_data(&MPI_Send_stats[current_caliper][dest], type, count);

    return rc;
}

void printReadable(const char *title, struct p2p_info **trace, int swap)
{
    printf("%s", title);
    for (int x = 0; x < num_tasks; x++)
        for (int y = 0; y < num_tasks; y++)
            if (trace[x][y].num_calls > 0)
                 printf("Rank %d %s %d  %ld calls with %.3E %s\n",
                        x, swap ? "from" : "to", y,
                        trace[x][y].num_calls,
                        trace[x][y].num_bytes / BYTE_SCALE, BYTE_UNIT);
}

void printCSV(const char *title, struct p2p_info **trace)
{
    printf("%s", title);
    // Column names
    printf("\"\"");
    for (int y = 0; y < num_tasks; y++)
        printf(",\"%d\"", y);
    printf("\n");
    // Table content
    for (int x = 0; x < num_tasks; x++)
    {
        printf("\"%d\"", x);
        for (int y = 0; y < num_tasks; y++)
	    printf(",\"%.3E\"", trace[x][y].num_bytes / BYTE_SCALE);
	printf("\n");
    }
}

void updateGlobalStats(struct p2p_info **global, struct p2p_info **step)
{
    for (int x = 0; x < num_tasks; x++)
        for (int y = 0; y < num_tasks; y++)
            if (step[x][y].num_calls > 0)
            {
                global[x][y].num_calls += step[x][y].num_calls;
                global[x][y].num_bytes += step[x][y].num_bytes;
            }
}

extern "C" int __wrap_MPI_Finalize(void)
{
    int rc;
    MPI_Status status;

    struct p2p_info **MPI_Send_procs_stats,
      **MPI_Recv_procs_stats; // Stats by iteration
    struct p2p_info **MPI_Send_total_stats,
      **MPI_Recv_total_stats; // Total execution stats
    int i, j;

    tracing = 0; // Stop getting info.

    if (myrank == 0)
    {
        // Master collects stats and prints them
        MPI_Send_procs_stats = (struct p2p_info **)
                        malloc(num_tasks * sizeof(struct p2p_info *));
        MPI_Recv_procs_stats = (struct p2p_info **)
                        malloc(num_tasks * sizeof(struct p2p_info *));
        MPI_Send_total_stats = (struct p2p_info **)
                        malloc(num_tasks * sizeof(struct p2p_info *));
        MPI_Recv_total_stats = (struct p2p_info **)
                        malloc(num_tasks * sizeof(struct p2p_info *));

        for (i = 0; i < num_tasks; i++)
        {
            MPI_Send_procs_stats[i] = (struct p2p_info *)
                    malloc(num_tasks * sizeof(struct p2p_info));
            memset(MPI_Send_procs_stats[i], 0,
                    num_tasks * sizeof(struct p2p_info));

            MPI_Recv_procs_stats[i] = (struct p2p_info *)
                    malloc(num_tasks * sizeof(struct p2p_info));
            memset(MPI_Recv_procs_stats[i], 0,
                    num_tasks * sizeof(struct p2p_info));

            MPI_Send_total_stats[i] = (struct p2p_info *)
                    malloc(num_tasks * sizeof(struct p2p_info));
            memset(MPI_Send_total_stats[i], 0,
                    num_tasks*sizeof(struct p2p_info));

            MPI_Recv_total_stats[i] = (struct p2p_info *)
                    malloc(num_tasks * sizeof(struct p2p_info));
            memset(MPI_Recv_total_stats[i], 0,
                    num_tasks*sizeof(struct p2p_info));

        }

        printf("STATISTICS BY ITERATION\n");

        // For each iteration, master process gets the stats from all the other
        // processes master prints stats of each iteration and compute total
        // statistic at the same time.
        for (i = 0; i <= current_caliper; i++)
	{

            memcpy(MPI_Send_procs_stats[0], MPI_Send_stats[i],
                    sizeof(struct p2p_info)*num_tasks);
            memcpy(MPI_Recv_procs_stats[0], MPI_Recv_stats[i],
                    sizeof(struct p2p_info)*num_tasks);

            for (j = 1; j < num_tasks; j++)
            {
                __real_MPI_Recv(MPI_Send_procs_stats[j],
				2 * num_tasks, MPI_LONG, j,
				123, MPI_COMM_WORLD, &status);
		__real_MPI_Recv(MPI_Recv_procs_stats[j],
				2 * num_tasks, MPI_LONG, j,
				123, MPI_COMM_WORLD, &status);
            }

            // Printing the stats by iteration and calculating totals
            char title[256];

            sprintf(title, "ISEND CSV stats Iteration %d\n", i);
            printCSV(title, MPI_Send_procs_stats);
            sprintf(title, "ISEND stats Iteration %d\n", i);
            printReadable(title, MPI_Send_procs_stats, 0);
            updateGlobalStats(MPI_Send_total_stats, MPI_Send_procs_stats);

            sprintf(title, "RECV CSV stats Iteration %d\n", i);
            printCSV(title, MPI_Recv_procs_stats);
            sprintf(title, "RECV stats Iteration %d\n", i);
            printReadable(title, MPI_Recv_procs_stats, 1);
            updateGlobalStats(MPI_Recv_total_stats, MPI_Recv_procs_stats);
        }

        printCSV("ISEND TOTAL CSV STATISTICS\n", MPI_Send_total_stats);
        printReadable("ISEND TOTAL STATISTICS\n", MPI_Send_total_stats, 0);

        printCSV("RECV TOTAL CSV STATISTICS\n", MPI_Recv_total_stats);
        printReadable("RECV TOTAL STATISTICS\n", MPI_Recv_total_stats, 1);
    }
    else
    {
        // Other processes send the stats to the master process
        for (i = 0; i <= current_caliper; i++)
        {
	    __real_MPI_Send(MPI_Send_stats[i], 2 * num_tasks, MPI_LONG, 0, 123,
			    MPI_COMM_WORLD);
	    __real_MPI_Send(MPI_Recv_stats[i], 2 * num_tasks, MPI_LONG, 0, 123,
			    MPI_COMM_WORLD);
	}
    }

    rc = __real_MPI_Finalize();
    return rc;
}
