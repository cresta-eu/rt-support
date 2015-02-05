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
 * graph
 * Contains functions to create a target graph in the "deco 0" file format. See
 * Scotch user ref for more information regarding the file format.
 *
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <scotch.h>
#include <math.h>

#include "topology.h"
#include "graph.h"
#include "proc_map.h"

//#define DEBUG

/* Estimations gotten from measurements on the Lindgren cluster */
/*
static comm_lat lindgren_lat =
{
	.intra_numa = 55,
	.intra_cpu = 71,
	.intra_node = 72,
	.intra_nic = 140,
	.inter_node_fix = 140,
	.inter_node_hop = 12
};
*/

/*
 * get_array_size
 * Gives the number of elements required in an array, for it to be able to store distances between
 * all nodes in a set. formula (pids^2)-pids / 2 comes from the full matrix, excluding diagonal, split in two.
*/
int get_array_size(int pids)
{
	return (pow(pids,2) - pids ) / 2;
}

/*
 * get_core_offset
 * returns the cores "local core number" in its node.
 *
*/
int get_core_offset(int pid, int core_per_node)
{
	return pid % core_per_node;
}


/*
 * set_node_local_distances
 * Updates the array pointed to by the first argument with the distance between some core 1 and all following cores that is local to the node of core 1.
 * As all cores are local to the node, we only need to know the cores offset in the nodet (and not its global process id), so the passed pointer
 * points to the first index to update (passed core's id +1).
*/
void set_node_local_distances(comm_lat *latencies, int* distances, int start_index, int core_offset, const hw_topology* hw_top)
{
	int current_cpu, cpu_offset, current_numa, numa_offset, local_numa, local_cpu;
	int i,last;
	//find which CPU and NUMA-node the core is in, and its offset in respective unit.
	current_cpu = core_offset / hw_top->core_per_cpu;
	cpu_offset = core_offset - (current_cpu*hw_top->core_per_cpu);
	current_numa = cpu_offset / hw_top->core_per_numa;
	numa_offset = cpu_offset - (current_numa*hw_top->core_per_numa);
	local_numa = hw_top->core_per_numa - numa_offset;
	local_cpu = hw_top->core_per_cpu - cpu_offset;

	last = hw_top->core_per_node - core_offset-1;
	for(i=0;i<last;i++)
	{
		#ifdef DEBUG
		printf("\tcomparing with: %d", core_offset+i);
		#endif
		if(i<local_numa)
		{
			distances[start_index + i] = latencies->intra_numa;
			#ifdef DEBUG
			printf("\t local numa!\n");
			#endif
		}
		else if(i<local_cpu)
		{
			distances[start_index + i] = latencies->intra_cpu;
			#ifdef DEBUG
			printf("\t local cpu!\n");
			#endif
		}
		else
		{
			distances[start_index + i] = latencies->intra_node;
			#ifdef DEBUG
			printf("\t local node\n");
			#endif
		}
	}
	return;
}

/*
 * set_node_external_distances
 * Updates the array pointed to by the first argument with the distance between some core and all cores in a different node.
 * As the distance between the core and all cores in the external node are equal, we simply take as argument the 'start_index'
 * of the array for the first write, and the number of nodes to update, along with of course the nid of the two nodes.
 *
*/
void set_node_external_distances(comm_lat *latencies, int* distances, int start_index, int nid_1, int nid_2,  const hw_topology* hw_top)
{
	int i, max, latency;

	// Update distances [start_index] to [start_index+hw_top->cores_per_node] with the distance.
	latency = get_inter_node_lat(latencies, &hw_top->max_dim, nid_1, nid_2);
	max = hw_top->core_per_node;
	for(i=0;i<max;i++)
	{
		distances[start_index + i] = latency;
	}
}

graph_t* get_subgraph_from_supergraph(const graph_t *super_graph, size_t vertex_list_size, const int *vertex_list, int pid)
{
#ifdef DEBUG
	printf("getting sub_graph_from_supergraph\n");
#endif
	SCOTCH_Num super_vertnbr, super_edgenbr;
	SCOTCH_Num *super_velotab, *super_verttab, *super_edgetab, *super_edlotab;
	graph_t *sub_graph;
	SCOTCH_Num sub_vertnbr, sub_edgenbr;
	SCOTCH_Num *sub_velotab, *sub_vendtab, *sub_vertlbl, *sub_verttab, *sub_edgetab, *sub_edlotab;

	if((sub_graph = allocate_graph()) != NULL)
	{
		SCOTCH_graphData(super_graph,
					NULL,
					&super_vertnbr,
					&super_verttab,
					NULL,
					&super_velotab,
					NULL,
					&super_edgenbr,
					&super_edgetab,
					&super_edlotab);
#ifdef DEBUG
		printf("loaded graph with %d (%zu) vertices and %d edges\n", super_vertnbr, vertex_list_size, super_edgenbr);
#endif
		sub_vertnbr = vertex_list_size;
		if(NULL != (sub_vertlbl = malloc(sizeof(SCOTCH_Num) * sub_vertnbr)))
		{
			if(NULL != (sub_verttab = malloc(sizeof(SCOTCH_Num) * (sub_vertnbr + 1))))
			{
				sub_edgenbr = get_num_subgraphedges_from_supergraph(super_verttab, super_edgetab, vertex_list_size, vertex_list);
				if(NULL != (sub_edgetab = malloc(sizeof(SCOTCH_Num) * sub_edgenbr)))
				{
					if(NULL != (sub_edlotab = malloc(sizeof(SCOTCH_Num) * sub_edgenbr)))
					{
						if(NULL != (sub_velotab = malloc(sizeof(SCOTCH_Num) * sub_vertnbr)))
						{
							set_subgraph_from_supergraph(&sub_vertlbl, &sub_vendtab, &sub_verttab, &sub_edgetab, &sub_edlotab, &sub_velotab, super_verttab, super_edgetab, super_edlotab, super_velotab, vertex_list_size, vertex_list, super_vertnbr, pid);
							if(0 == SCOTCH_graphBuild(sub_graph,
										0,
										sub_vertnbr,
										sub_verttab,
										sub_vendtab,
										sub_velotab,
										sub_vertlbl,
										sub_edgenbr,
										sub_edgetab,
										sub_edlotab))
							{
#ifdef DBEUG
								printf("%d: Successfully created subgraph from supergraph!\n", pid);
#endif
								return sub_graph;
							}
							free(sub_velotab);
						}
						free(sub_edlotab);
					}
					free(sub_edgetab);
				}
				free(sub_verttab);
			}
			free(sub_vertlbl);
		}
		free_graph(sub_graph);
	}
	fprintf(stderr, "Could not build subgraph from supergraph!\n");
	return NULL;
}

void set_subgraph_from_supergraph(SCOTCH_Num **sub_vertlbl, SCOTCH_Num **sub_vendtab, SCOTCH_Num **sub_verttab, SCOTCH_Num **sub_edgetab, SCOTCH_Num **sub_edlotab, SCOTCH_Num **sub_velotab, const SCOTCH_Num *super_verttab, const SCOTCH_Num *super_edgetab, const SCOTCH_Num *super_edlotab, const SCOTCH_Num *super_velotab, int num_subgraph_vertices, const int *subgraph_vertex_list, int num_vertices, int pid)
{
	int i, j, num_edges, current_vertex, edgetab_index;
	int *lookup_table;

	if(NULL != (lookup_table = malloc(num_vertices * sizeof(int))))
	{
		for(i = 0; i < num_subgraph_vertices; i++)
		{
			//printf("setting lookup %d -> %d\n", subgraph_vertex_list[i], i);
			if(subgraph_vertex_list[i] > num_vertices)
			{
				printf("trouble!\n");
			}
			lookup_table[subgraph_vertex_list[i]] = i;
		}

		edgetab_index = 0;
		*sub_vendtab = *sub_verttab + 1;
		for(i = j = 0; i < num_subgraph_vertices; i++)
		{
			current_vertex = subgraph_vertex_list[i];
			(*sub_vertlbl)[i] = i;
			(*sub_verttab)[i] = edgetab_index;
			if(NULL != super_velotab)
			{
				(*sub_velotab)[i] = super_velotab[current_vertex];
			} else
			{
				(*sub_velotab)[i] = 1; /* if no loads are set on supergraph's vertices -assume equal load */
			}

			num_edges = super_verttab[current_vertex + 1] - super_verttab[current_vertex];

			if(NULL != super_edlotab)
			{
				for(j = 0; j < num_edges; j++)
				{
					if(vertex_is_in_list(super_edgetab[super_verttab[current_vertex]+j], num_subgraph_vertices, subgraph_vertex_list))
					{
						if(super_edgetab[super_verttab[current_vertex]+j] > num_vertices)
						{
							printf("trouble!\n");
						}
						(*sub_edgetab)[edgetab_index] = lookup_table[super_edgetab[super_verttab[current_vertex]+j]];
						(*sub_edlotab)[edgetab_index] = super_edlotab[super_verttab[current_vertex]+j];
						edgetab_index++;
					}
				}

			} else
			{ /* if no loads are set on supergraph's edges -assume equal load */
				for(j = 0; j < num_edges; j++)
				{
					if(vertex_is_in_list(super_edgetab[super_verttab[current_vertex]+j], num_subgraph_vertices, subgraph_vertex_list))
					{
						if(super_edgetab[super_verttab[current_vertex]+j] > num_vertices)
						{
							printf("trouble!\n");
						}
						(*sub_edgetab)[edgetab_index] = lookup_table[super_edgetab[super_verttab[current_vertex]+j]];
						(*sub_edlotab)[edgetab_index] = 1;
						edgetab_index++;
					}
				}
			}
		}
		(*sub_verttab)[num_subgraph_vertices] = edgetab_index;
		free(lookup_table);
	} else
	{
		fprintf(stderr, "could not allocate memory for lookup table!\n");
	}
}

SCOTCH_Num get_num_subgraphedges_from_supergraph(const SCOTCH_Num *super_verttab, const SCOTCH_Num *super_edgetab, int num_subgraph_vertices, const int *subgraph_vertex_list){
	SCOTCH_Num sub_edgenbr;
	int i, j, num_edges, current_vertex;

	sub_edgenbr = 0;
	for(i = 0; i < num_subgraph_vertices; i++)
	{
		current_vertex = subgraph_vertex_list[i];
		num_edges = super_verttab[current_vertex + 1] - super_verttab[current_vertex];
		for(j = 0; j < num_edges; j++)
		{
			if(vertex_is_in_list(super_edgetab[super_verttab[current_vertex]+j], num_subgraph_vertices, subgraph_vertex_list))
			{
				sub_edgenbr++;
			}
		}
	}
	return sub_edgenbr;
}

/*TODO: Optimization: have the vertex_list sorted... */
int vertex_is_in_list(int vertex, size_t list_size, const int *vertex_list)
{
	size_t i;
	for(i = 0; i < list_size; i++)
	{
		if(vertex_list[i] == vertex)
			return 1;
	}
	return 0;
}

void get_node_local_vertices_from_mapping(const mapping_t *mapping, int node_number, int *res)
{
	int i, j;

	for(i = j =0; i < mapping->size; i++)
	{
		if(mapping->mapping[i] == node_number)
			res[j++] = i;
	}
}

int num_node_local_vertices_from_mapping(const mapping_t *mapping, int node_number)
{
	int i, res;

	for(i = res = 0; i < mapping->size; i++)
	{
		if(mapping->mapping[i] == node_number)
			res++;
	}
	return res;
}

