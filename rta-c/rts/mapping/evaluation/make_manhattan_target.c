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

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "proc_map.h"
#include "scotch.h"
#include "measurement.h"
#include "proc_map_types.h"

int main(int argc, char **argv)
{
	int pid;
	comm_lat latencies;
	hw_topology lindgren_hw =
	{
		.core_per_numa = 6,
		.numa_per_cpu = 2,
		.cpu_per_node = 2,
		.core_per_cpu = 12,
		.core_per_node = 24,
		.node_per_nic = 2,
	};

	arch_t *target_graph = NULL;
	char *target_arch_filename = NULL;

	MPI_Init(0,0);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &lindgren_hw.reserved_cores);
	if(argc < 2 || argc > 3)
	{
		if(pid == 0)
		{
			printf("usage: %s <target file>\n", argv[0]);
			printf("or:\n");
			printf("usage: %s <target file> <num cores / node>\n", argv[0]);
		}
		return 0;
	} else
	{
		target_arch_filename = argv[1];
		if(argc == 3)
		{
			lindgren_hw.core_per_node = atoi(argv[2]);
			lindgren_hw.core_per_cpu = lindgren_hw.core_per_node / 2;
			lindgren_hw.core_per_numa = lindgren_hw.core_per_cpu / 2;
		}
	}

	if(TOP_SUCC != get_topo_max_dim(&(lindgren_hw.max_dim)))
	{
		fprintf(stderr, "couldn't get max dimensions for network topology\n");
		return -1;
	}

	double start_time, stop_time;

	do{
		start_time = MPI_Wtime();
		measure_manhattan_distance(&lindgren_hw, &latencies);
		stop_time = MPI_Wtime();
	} while((latencies.inter_node_fix < 0) || (latencies.inter_node_hop < 0));
	if(pid ==0)
	{
		printf("inter node fix: %d\n", latencies.inter_node_fix);
		printf("inter node hop: %d\n", latencies.inter_node_hop);
		printf("measure manhattan time: %f\n", stop_time - start_time);

	}
	start_time = MPI_Wtime();
	target_graph = gen_target_manhattan(&lindgren_hw, pid, &latencies);
	stop_time = MPI_Wtime();
	if(pid==0)
	{
		printf("generate manhattan time: %f\n", stop_time - start_time);
	}
	if(pid== 0)
	{
		FILE *fp;
		fp = fopen(target_arch_filename, "w");
		SCOTCH_archSave(target_graph, fp);
		fclose(fp);

		free_arch(target_graph);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}

