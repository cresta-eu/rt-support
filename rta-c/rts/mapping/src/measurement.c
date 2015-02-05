/*
 * rta-c - runtime system administration component.
 *  Copyright (C) 2014  Sebastian Tunstig.
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

/*
 * measurement.c
 * Makes measurements of latency between processor cores in a Cray cluster.
 * Implementation is specific for the Lindgren system at PDC/KTH, Stockholm.
 * System properties include 12 core CPUs with two NUMA-nodes of 6 cores per
 * CPU, two CPUs per node and two nodes per network interface. The network
 * topology is a Cray Gemini interconnect (3D Torus).
 * Communication latencies are not tested between every core, rather by one core
 * (pid 0, abbreviated 'master' sometimes) to every other core.
 *
 * Author: Sebastian Tunstig, tunstig@kth.se
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <mpi.h>
#include <math.h>

#include "measurement.h"
#include "definitions.h"
#include "proc_map_types.h"
#include "proc_map.h"
#include "topology.h"
#include "least_squares.h"

/*
 * measure_manhattan_distance
 * TODO: doc
 */
map_status_t measure_manhattan_distance(const hw_topology *hw_top, comm_lat *latencies)
{
	map_status_t res;
	MPI_Status status;
    int pid,nid,num_proc, num_ext_nodes;
	int connected_pid_1, connected_pid_2;
	int inter_gemini_dist = UNSET;
	mesh_coord_t local_pos;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

	num_ext_nodes = (int) ceil((double) num_proc / hw_top->core_per_node) - 1;
	if(num_ext_nodes == 0)
	{
		return MAP_ARG_ERROR;
	}
	if(TOP_FAIL == get_nid_from_pid(pid, &nid))
    {
        fprintf(stderr, "Couldn't retreieve nid from pid\n");
        return MAP_LIB_ERROR;
    }
	if(TOP_FAIL == get_pos_from_nid(nid, &local_pos))
	{
		fprintf(stderr, "couldn't retreive position from nid\n");
		return MAP_LIB_ERROR;
	}
	/* Find the first (if any) occurences of nodes sharing gemini-node */
	find_connected_nodes(hw_top, num_ext_nodes, &connected_pid_1, &connected_pid_2);
	MPI_Barrier(MPI_COMM_WORLD);
	if(connected_pid_1 == pid)
	{
		respond_to_ping(connected_pid_2, MEASUREMENT_ITERATIONS);
	} else if(connected_pid_2 == pid)
	{
		if(pid != 0)
		{
			inter_gemini_dist = (int) (send_ping(connected_pid_1, MEASUREMENT_ITERATIONS) * MEASUREMENT_CLOCK_MULTIPLIER);
			MPI_Send(&inter_gemini_dist, 1, MPI_REAL, 0, MPI_TAG, MPI_COMM_WORLD);
		}
	}

    if(pid == 0)
    {
		int *nids;
		int *ext_time_res;
		int *num_hops;
		int current_nid = nid;
		int i;
		mesh_coord_t current_pos;

		#ifdef DEBUG
		printf("---\nmaster pid on nid: %d\nnum ext nodes: %d\nconnected_pid_2: %d\n---\n", nid, num_ext_nodes, connected_pid_2);
		#endif
		if(connected_pid_2 != pid)
		{
			MPI_Recv(&inter_gemini_dist, 1, MPI_REAL, connected_pid_2, MPI_TAG, MPI_COMM_WORLD, &status);
		}
		#ifdef DEBUG
		printf("inter_gemini_dist: %d\n", inter_gemini_dist);
		#endif
		/* ping every first process of every new node and retreive its node id.*/
		nids = (int*) malloc(num_ext_nodes * sizeof(int));
		ext_time_res = (int*) malloc(num_ext_nodes * sizeof(double));
		num_hops = (int*) malloc(num_ext_nodes * sizeof(int));
    	MPI_Barrier(MPI_COMM_WORLD);
		for(i = 1;i<=num_ext_nodes;i++)
		{
			if(TOP_FAIL == get_nid_from_pid(i * hw_top->core_per_node, &current_nid))
			{
				fprintf(stderr, "couldn't retreive nid from pid\n");
				free(nids);
				free(ext_time_res);
				return MAP_LIB_ERROR;
			}
			if(TOP_FAIL == get_pos_from_nid(current_nid, &current_pos))
			{
				fprintf(stderr, "couldn't retreive position from nid\n");
				return MAP_LIB_ERROR;
			}
			num_hops[i - 1] = get_torus_mesh_manhattan_distance(&(hw_top->max_dim), &local_pos, &current_pos);
			#ifdef DEBUG
			printf("Pinging pid: %d of new node %d. hops: %d. (%d, %d, %d) <-> (%d, %d, %d)\n", i * hw_top->core_per_node, current_nid, num_hops[i - 1], local_pos.mesh_x, local_pos.mesh_y, local_pos.mesh_z, current_pos.mesh_x, current_pos.mesh_y, current_pos.mesh_z);
			#endif
			nids[i - 1] = current_nid;
			ext_time_res[i - 1] = (int) (send_ping((i * hw_top->core_per_node), MEASUREMENT_ITERATIONS) * MEASUREMENT_CLOCK_MULTIPLIER);
			#ifdef DEBUG
			printf("distance: %d\n", ext_time_res[i - 1]);
			#endif
		}
		//print_res(nid, num_local_proc, local_time_res, num_ext_nodes, nids, ext_time_res);
		res = set_manhattan_inter_node_distances(&latencies->inter_node_fix, &latencies->inter_node_hop,
												ext_time_res, num_hops, num_ext_nodes);
		free(nids);
		free(num_hops);
		free(ext_time_res);
	} else /* (pid != 0) */
    {
		/* find node id of the predecessing process */
		int predecessing_pid_nid;
		if(TOP_FAIL == get_nid_from_pid(pid - 1, &predecessing_pid_nid))
		{
			fprintf(stderr, "Couldn't retreieve nid from pid\n");
			res = MAP_LIB_ERROR;
		}
		else if(predecessing_pid_nid != nid) /* local process is the first process of a node (that is not the first node) */
		{
			#ifdef DEBUG
			printf("pid: %d responding to pid 0\n", pid);
			#endif
			MPI_Barrier(MPI_COMM_WORLD);
			respond_to_ping(0, MEASUREMENT_ITERATIONS);
			res = MAP_SUCCESS;
		} else /* No pings to be sent/received */
		{
			res = MAP_SUCCESS;
    		MPI_Barrier(MPI_COMM_WORLD);
		}
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return res;
}


map_status_t measure_intra_node_distances(const hw_topology *hw_top, comm_lat *latencies)
{
	//printf("measuring intra-node distances!\n");
	int pid, nid, master_nid, num_proc;
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

	if(hw_top->core_per_node < 2 || hw_top->reserved_cores < 2)
		return MAP_SUCCESS;

	if(TOP_FAIL == get_nid_from_pid(pid, &nid))
	{
		fprintf(stderr, "Couldn't retreieve nid from pid\n");
		return MAP_LIB_ERROR;
	}
	if(TOP_FAIL == get_nid_from_pid(0, &master_nid))
	{
		fprintf(stderr, "Couldn't retreieve nid from pid\n");
		return MAP_LIB_ERROR;
	}

	if(pid % hw_top->core_per_node == 0)
	{
		double *local_time_res;
		int i;
		int local_cores = hw_top->reserved_cores < hw_top->core_per_node ? hw_top->reserved_cores : hw_top->core_per_node;
		local_time_res = (double*) malloc((local_cores - 1) * sizeof(double));
		MPI_Barrier(MPI_COMM_WORLD);

		/* ping every local node */
		for(i=1;i<local_cores;i++)
		{
			//printf("pinging local core: %d\n", pid+i);
			local_time_res[i - 1] = send_ping(pid+i, MEASUREMENT_ITERATIONS);
		}
		/* update hw_top->latencies with the measured data as basis */
		set_manhattan_intra_node_distances(hw_top, latencies, local_time_res);
		free(local_time_res);
	} else
	{
		MPI_Barrier(MPI_COMM_WORLD);
		respond_to_ping(pid - (pid % hw_top->core_per_node), MEASUREMENT_ITERATIONS);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	return MAP_SUCCESS;
}

map_status_t set_manhattan_inter_node_distances(int *fix, int *hop, int *ext_time_res, int *num_hops, int num_ext_nodes)
{
	map_status_t result;
	int **measurements;
	int **results;
	int residual_sq_sum;
	int i;
	#ifdef DEBUG
	printf("updating inter_node distance records with %d equations\n", num_ext_nodes);
	#endif
	if(num_ext_nodes < 2)
	{
		fprintf(stderr, "Too few inter-node measurements made. Need atleast measurements from 2 nodes (not counting the local node of the first process)\n");
		return MAP_ARG_ERROR;
	} else if (num_ext_nodes == 2) /* The equation has exactly one solution */
	{
	#ifdef DEBUG
		printf("!!! Measuring distances based on minimum data, consider adding more nodes to get better results!\n");
	#endif
	}

	/* prepare data for least_squares */
	measurements = (int**) malloc(3 * sizeof(int*));
	results = (int**) malloc(2 * sizeof(int*));
	if(measurements == NULL || results == NULL)
	{
		fprintf(stderr, "could not allocate memory\n");
		return MAP_IO_ERROR;
	}
	measurements[0] = malloc(num_ext_nodes * sizeof(int));
	if(measurements[0] == NULL)
	{
		fprintf(stderr, "could not allocate memory\n");
		return MAP_IO_ERROR;
	}
	for(i = 0;i < num_ext_nodes;i++) /* All measurements have the fix cost of inter-node communication  */
	{
		measurements[0][i] = 1;
	}
	measurements[1] = num_hops;
	measurements[2] = ext_time_res;

	results[0] = fix;
	results[1] = hop;
	if(0 == least_squares(num_ext_nodes, (const int**) measurements, results, &residual_sq_sum))
	{
		/* printf("results\n---\nhop: %d\nfix: %d\nres-sum: %d\n", *hop, *fix, residual_sq_sum); */
		result = MAP_SUCCESS;

	} else
	{
	/*
		TODO: Temporary fix!
	*/
		*fix = 200;
		*hop = 20;
		result = MAP_SUCCESS;
		//result = MAP_ARG_ERROR;
	}
	free(measurements[0]);
	free(measurements);
	free(results);
	return result;
}

void set_manhattan_intra_node_distances(const hw_topology *hw_top, comm_lat *latencies, double *local_time_res)
{
	int i;
	int range, offset;
	double sum = 0;

	/* compute intra-numa average */
	range = hw_top->reserved_cores < hw_top->core_per_numa ? hw_top->reserved_cores - 1 : hw_top->core_per_numa - 1;
	for(i=0;i < range; i++)
	{
		sum += local_time_res[i];
	}
	sum /= range;
	latencies->intra_numa = (int) (sum * MEASUREMENT_CLOCK_MULTIPLIER);
	/* printf("latencies: intra_numa: %d\n", latencies->intra_numa); */
	if(hw_top->reserved_cores <= hw_top->core_per_numa)
		return;

	/* compute intra_cpu average */
	sum = 0;
	offset = range;
	range = hw_top->reserved_cores < hw_top->core_per_cpu ? (hw_top->reserved_cores - hw_top->core_per_numa) : (hw_top->core_per_cpu - hw_top->core_per_numa);
	for(i = 0; i < range; i++)
	{
		sum += local_time_res[offset+i];
	}
	sum /= range;
	latencies->intra_cpu = (int) (sum * MEASUREMENT_CLOCK_MULTIPLIER);
	/* printf("latencies: intra_cpu: %d\n", latencies->intra_cpu); */
	if(hw_top->reserved_cores <= hw_top->core_per_cpu)
		return;

	/* compute intra_node average */
	sum = 0;
	offset = range;
	range = hw_top->reserved_cores < hw_top->core_per_node ? hw_top->reserved_cores - hw_top->core_per_cpu : hw_top->core_per_node - hw_top->core_per_cpu;
	for(i = 0; i < range; i++)
	{
		sum += local_time_res[offset+i];
	}
	sum /= range;
	latencies->intra_node = (int) (sum * MEASUREMENT_CLOCK_MULTIPLIER);
	/* printf("latencies: intra_node %d\n", latencies->intra_node); */
}

/*
* find_connected_nodes
* TODO: doc
*/
void find_connected_nodes(const hw_topology *hw_top, int num_ext_nodes, int *pid_1, int *pid_2)
{
	int i;
	mesh_coord_t prior_pos, current_pos;

	if(TOP_FAIL == get_pos_from_pid(0, &prior_pos))
	{
		fprintf(stderr, "couldn't retreive position from nid\n");
		return;
	}
	for(i = 1;i < num_ext_nodes; i++)
	{
		if(TOP_FAIL == get_pos_from_pid(i * hw_top->core_per_node, &current_pos))
		{
			fprintf(stderr, "couldn't retreive position from nid\n");
			return;
		}

		if(mesh_coord_t_equals(&current_pos, &prior_pos))
		{
			*pid_1 = (i - 1) * hw_top->core_per_node;
			*pid_2 = i * hw_top->core_per_node;
			return;
		}
		prior_pos = current_pos;
	}
	/* found none */
	*pid_1 = UNSET;
	*pid_2 = UNSET;
	return;
}


void respond_to_ping(int pid, int iterations)
{
	int i, buffer;
	MPI_Status status;

	/* sync peers before ping test (barrier)*/
	MPI_Recv(&buffer, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD, &status);
	MPI_Send(&i, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD);
	for(i=0;i<iterations;i++)
	{
		MPI_Recv(&buffer, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD, &status);
		MPI_Send(&i, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD);
	}
	return;
}

double send_ping(int pid, int iterations)
{
	int i,buffer;
	double start_time, stop_time;
	MPI_Status status;

	/* sync peers before ping test (barrier) */
    MPI_Send(&i, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD);
	MPI_Recv(&buffer, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD, &status);
	/* perform ping-test */
	start_time = MPI_Wtime();
	for(i=0;i<iterations;i++)
	{
       	MPI_Send(&i, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD);
		MPI_Recv(&buffer, 1, MPI_REAL, pid, MPI_TAG, MPI_COMM_WORLD, &status);
    }
	stop_time = MPI_Wtime();
	return stop_time - start_time;
}

