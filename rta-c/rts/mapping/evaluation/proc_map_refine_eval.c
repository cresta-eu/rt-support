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


int main(int argc, char **argv){
	int pid;
	int i;
	int num_source_graph_vertices = 0;
	map_status_t status;
	comm_lat latencies =
	{
		.intra_numa = 116,
		.intra_cpu = 148,
		.intra_node = 150,
	};
	hw_topology lindgren_hw =
	{
		.core_per_numa = 6,
		.numa_per_cpu = 2,
		.cpu_per_node = 2,
		.core_per_cpu = 12,
		.core_per_node = 24,
		.node_per_nic = 2,
	};

	arch_t *target_arch = NULL;
	graph_t *source_graph = NULL;
	char *source_graph_filename = NULL;
	char *target_arch_filename = NULL;
	char *old_mapping_filename = NULL;
	char *new_mapping_filename = NULL;
	mapping_t *old_mapping = NULL;
	mapping_t *new_mapping = NULL;

	MPI_Init(0,0);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &lindgren_hw.reserved_cores);
	if(argc < 5 || argc > 6)
	{
		if(pid == 0)
		{
			printf("usage: %s <source file> <arch file> <old mapping file> <new mapping file>\n", argv[0]);
			printf("or:\n");
			printf("usage: %s <source file> <arch file> <old mapping file> <new mapping file> <num cores / node>\n", argv[0]);
		}
		MPI_Finalize();
		return 0;
	} else
	{
		source_graph_filename = argv[1];
		target_arch_filename = argv[2];
		old_mapping_filename = argv[3];
		new_mapping_filename = argv[4];
		if(argc==6)
		{
			lindgren_hw.core_per_node = atoi(argv[5]);
			lindgren_hw.core_per_cpu = lindgren_hw.core_per_node / 2;
			lindgren_hw.core_per_numa = lindgren_hw.core_per_cpu / 2;
		}
	}
	if((pid % lindgren_hw.core_per_node) == 0)
	{
		int nid;
		mesh_coord_t pos;

		if(TOP_FAIL == get_nid_from_pid(pid, &nid))
		{
			fprintf(stderr, "Couldn't retreieve nid from pid\n");
			return MAP_LIB_ERROR;
		}
		if(TOP_FAIL == get_pos_from_nid(nid, &pos))
		{
			fprintf(stderr, "couldn't retreive position from nid\n");
			return MAP_LIB_ERROR;
		}
		printf("nid %d on pos (%d, %d, %d)\n", nid, pos.mesh_x, pos.mesh_y, pos.mesh_z);
	}
	if(TOP_SUCC != get_topo_max_dim(&(lindgren_hw.max_dim)))
	{
		fprintf(stderr, "couldn't get max dimensions for network topology\n");
		MPI_Finalize();
		return -1;
	}
	if(NULL == (source_graph = load_graph(source_graph_filename)))
	{
		fprintf(stderr, "couldn't load source-graph from file, exiting!\n");
		MPI_Finalize();
		return -1;
	}
	SCOTCH_graphSize(source_graph, &num_source_graph_vertices, NULL);

	if(pid == 0)
	{
		if(NULL == (target_arch = load_arch(target_arch_filename)))
		{
			fprintf(stderr, "couldn't load arch-graph from file, exiting!\n");
			return -1;
		}

		old_mapping = allocate_mapping(num_source_graph_vertices);

		if(-1 == load_mapping(old_mapping_filename, old_mapping))
		{
			fprintf(stderr, "couldn't load old mapping from file, exiting!\n");
			return -1;
		}
	} else if(pid % lindgren_hw.core_per_node == 0)
	{
		old_mapping = allocate_mapping(num_source_graph_vertices);
	}

	if((status = gen_comm_lat(&lindgren_hw, &latencies)) != MAP_SUCCESS)
	{
		fprintf(stderr, "problem building decomposition matrix!\n");
		MPI_Finalize();
		return 0;
	}

/*
	if(pid== 0)
	{
		printf("intra numa %d\n", latencies.intra_numa);
		printf("inter numa %d\n", latencies.intra_cpu);
		printf("inter cpu %d\n", latencies.intra_node);
	}
*/
	new_mapping = allocate_mapping(num_source_graph_vertices);

	/*
	   perform refinement
	 */
	 double start_time, stop_time;
	 start_time = MPI_Wtime();
	proc_map_refine(&lindgren_hw, &latencies, source_graph, target_arch, old_mapping, new_mapping);
	 stop_time = MPI_Wtime();
	 if(pid ==0)
	 {
	 	printf("refinement time: %f\n", stop_time - start_time);
	 }

	if(pid == 0)
	{
		FILE *fp;
		fp = fopen(new_mapping_filename, "w");
		for(i = 0; i < new_mapping->size; i++)
		{
			fprintf(fp, "%d %d\n", i, new_mapping->mapping[i]);
		}
		fclose(fp);

	/*
		FILE *fp;
		fp = fopen(new_mapping_filename, "w");
		for(i = 0; i < new_mapping->size; i++)
		{
			fprintf(fp, "%d %d\n", i, new_mapping->mapping[i] / lindgren_hw.core_per_node);
		}
		fclose(fp);
	*/
	}

	free_mapping(new_mapping);
	free_graph(source_graph);
	if(pid == 0)
	{
		free_arch(target_arch);
		free_mapping(old_mapping);
	} else if(pid % lindgren_hw.core_per_node == 0)
	{
		free_mapping(old_mapping);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}

