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

#include <stdlib.h>
#include <stdio.h>
#include <scotch.h>
#include <mpi.h>
#include <math.h>
#include "definitions.h"
#include "proc_map_types.h"
#include "proc_map.h"
#include "topology.h"
#include "measurement.h"
#include "graph.h"

/*
 * proc_map
 * performs process mapping by first mapping processes to nodes
 * when node-mappings are complete, the first mpi-process in every node
 * gets the global node-mapping data and performs local mappings to computational cores.
 * the final data is then collected by mpi process 0 before its distributed to all mpi processes.
 */
map_status_t proc_map(const hw_topology *hw_top, const comm_lat *latencies, graph_t *source_graph, arch_t *target_graph, mapping_t *mapping)
{
	int i, pid, num_proc, num_nodes, node_number, is_nodeleader, role_key, role_pid, sum;
	int *gatherv_displacements = NULL;
	mapping_t *node_local_mappings = NULL;
	mapping_t *node_mappings = NULL;
	mapping_t *gathered_mappings = NULL;
	SCOTCH_Num num_source_graph_vertices;
	SCOTCH_Num *num_node_local_mappings = NULL;
	map_status_t status;
	MPI_Comm role_comm;

	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	is_nodeleader = pid % hw_top->core_per_node == 0;
	if((pid % hw_top->core_per_node) == 0)
	{
		role_key = pid / hw_top->core_per_node;
	} else
	{
		role_key = pid - (pid * (pid / hw_top->core_per_node));
	}
	node_number = pid / hw_top->core_per_node;
	MPI_Comm_split(MPI_COMM_WORLD, is_nodeleader, role_key, &role_comm);
	MPI_Comm_rank(role_comm, &role_pid);

	/* TODO: Fix non-leader role_pids (not correct atm, but doesn't matter) */
	num_source_graph_vertices = graph_size(source_graph);
	num_nodes = (int) ceil((double)num_proc / hw_top->core_per_node);
	status = MAP_ERROR;
	if(is_nodeleader) /*if first core in node*/
	{
		if(NULL != (node_mappings = allocate_mapping(num_source_graph_vertices)))
		{
			if(pid == 0)
			{
#ifdef DBEUG
				printf("performing process mapping...");
#endif
				if(MAP_SUCCESS != (status = get_mapping(source_graph, target_graph, node_mappings)))
				{
#ifdef DEBUG
					fprintf(stderr, "Failed\n");
#endif
					return MAP_LIB_ERROR;
				} else
				{
#ifdef DEBUG
					printf("Node mapping successful. Continuing with core mappping\n");
#endif
				}
			}
			if(MPI_SUCCESS != (MPI_Bcast(node_mappings->mapping, node_mappings->size, MPI_INT, 0, role_comm)))
			{
				fprintf(stderr, "pid %d could not perform broadcast\n", pid);
				return MAP_IO_ERROR;
			}
		} else
		{
			return MAP_IO_ERROR;
		}

		node_local_mappings = allocate_mapping(num_node_local_vertices_from_mapping(node_mappings, node_number));
		if(MAP_SUCCESS != proc_map_node_local(hw_top, latencies, source_graph, node_mappings, node_local_mappings))
		{
			fprintf(stderr, "node local mapping did not complete, exiting\n");
			return -1;
		}

		/* retrieve node-level mappings and compute final mapping */
		if(pid == 0)
		{
			num_node_local_mappings = malloc(sizeof(SCOTCH_Num) * num_nodes);
		}
		if(MPI_SUCCESS != MPI_Gather(&(node_local_mappings->size), 1, MPI_INT, num_node_local_mappings, 1, MPI_INT, 0, role_comm))
		{
			fprintf(stderr, "error on MPI_Gather for pid: %d\n", pid);
			free(node_local_mappings);
			return MAP_IO_ERROR;
		}

		if(pid == 0)
		{
			gathered_mappings = allocate_mapping(num_source_graph_vertices);
			gatherv_displacements = malloc(sizeof(int) * num_nodes);
			gathered_mappings->mapping = malloc(sizeof(SCOTCH_Num) * gathered_mappings->size);
			sum = 0;
			for(i = 0; i < num_nodes; i++)
			{
				gatherv_displacements[i] = sum;
				sum += num_node_local_mappings[i];
			}
			if(MPI_SUCCESS != MPI_Gatherv(node_local_mappings->mapping, (int)node_local_mappings->size, MPI_INT, gathered_mappings->mapping, num_node_local_mappings, gatherv_displacements, MPI_INT, 0, role_comm))
			{
				fprintf(stderr, "error on MPI_Gatherv for pid: %d\n", pid);
				status = MAP_IO_ERROR;
			}
			aggregate_mapping_data(node_mappings, gatherv_displacements, gathered_mappings, mapping);
			free(gatherv_displacements);
			free_mapping(node_mappings);
			free_mapping(gathered_mappings);
		} else
		{
			if(MPI_SUCCESS != MPI_Gatherv(node_local_mappings->mapping, (int)node_local_mappings->size, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, role_comm))
			{
				fprintf(stderr, "error on MPI_Gatherv for pid: %d\n", pid);
			}
		}
		status = MAP_SUCCESS;
	}

	MPI_Bcast(mapping->mapping, mapping->size, MPI_INT, 0, MPI_COMM_WORLD);

	if(is_nodeleader)
	{
		free_mapping(node_local_mappings);
		if(pid == 0)
		{
			free(num_node_local_mappings);
			free_mapping(gathered_mappings);
		}
	}
	return MAP_SUCCESS;
}

map_status_t proc_map_refine(const hw_topology *hw_top,
		const comm_lat *latencies,
		graph_t *source_graph,
		arch_t *target_graph,
		mapping_t *prior_mapping,
		mapping_t *new_mapping)
{
	int i, pid, num_proc, num_nodes, node_number, is_nodeleader, role_key, role_pid, sum;
	int *gatherv_displacements = NULL;
	mapping_t *node_local_mappings = NULL;
	mapping_t *prior_node_mappings = NULL;
	mapping_t *new_node_mappings = NULL;
	mapping_t *gathered_mappings = NULL;
	SCOTCH_Num num_source_graph_vertices;
	SCOTCH_Num *num_node_local_mappings = NULL;
	MPI_Comm role_comm;

	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	is_nodeleader = pid % hw_top->core_per_node == 0;
	if((pid % hw_top->core_per_node) == 0)
	{
		role_key = pid / hw_top->core_per_node;
	} else
	{
		role_key = pid - (pid * (pid / hw_top->core_per_node));
	}
	node_number = pid / hw_top->core_per_node;
	MPI_Comm_split(MPI_COMM_WORLD, is_nodeleader, role_key, &role_comm);
	MPI_Comm_rank(role_comm, &role_pid);

	/* TODO: Fix non-leader role_pids (not correct atm, but doesn't matter) */
	num_source_graph_vertices = graph_size(source_graph);
	num_nodes = (int) ceil((double)num_proc / hw_top->core_per_node);
	if(is_nodeleader)
	{
		new_node_mappings = allocate_mapping(prior_mapping->size);
		if(pid == 0)
		{
			prior_node_mappings = allocate_mapping(prior_mapping->size);
			for(i = 0; i < prior_mapping->size; i++)
			{
				prior_node_mappings->mapping[i] = prior_mapping->mapping[i] / hw_top->core_per_node;
			}
			get_mapping_refinement(source_graph, target_graph, prior_node_mappings, new_node_mappings);
			free_mapping(prior_node_mappings);
		}

		if(MPI_SUCCESS != (MPI_Bcast(new_node_mappings->mapping, new_node_mappings->size, MPI_INT, 0, role_comm)))
		{
			fprintf(stderr, "pid %d could not perform broadcast\n", pid);
			return MAP_IO_ERROR;
		}
		node_local_mappings = allocate_mapping(num_node_local_vertices_from_mapping(new_node_mappings, node_number));
		if(MAP_SUCCESS != proc_map_node_local(hw_top, latencies, source_graph, new_node_mappings, node_local_mappings))
		{
			fprintf(stderr, "node local mapping did not complete, exiting\n");
			return -1;
		}

		/* retrieve node-level mappings and compute final mapping */
		if(pid == 0)
		{
			num_node_local_mappings = malloc(sizeof(SCOTCH_Num) * num_nodes);
		}
		if(MPI_SUCCESS != MPI_Gather(&(node_local_mappings->size), 1, MPI_INT, num_node_local_mappings, 1, MPI_INT, 0, role_comm))
		{
			fprintf(stderr, "error on MPI_Gather for pid: %d\n", pid);
			free(node_local_mappings);
			return MAP_IO_ERROR;
		}
		if(pid == 0)
		{
			gathered_mappings = allocate_mapping(num_source_graph_vertices);
			gatherv_displacements = malloc(sizeof(int) * num_nodes);
			gathered_mappings->mapping = malloc(sizeof(SCOTCH_Num) * gathered_mappings->size);
			sum = 0;
			for(i = 0; i < num_nodes; i++)
			{
				gatherv_displacements[i] = sum;
				sum += num_node_local_mappings[i];
			}
			if(MPI_SUCCESS != MPI_Gatherv(node_local_mappings->mapping, (int)node_local_mappings->size, MPI_INT, gathered_mappings->mapping, num_node_local_mappings, gatherv_displacements, MPI_INT, 0, role_comm))
			{
				fprintf(stderr, "error on MPI_Gatherv for pid: %d\n", pid);
				return MAP_IO_ERROR;
			}
			aggregate_mapping_data(new_node_mappings, gatherv_displacements, gathered_mappings, new_mapping);
			free(gatherv_displacements);
			free_mapping(new_node_mappings);
			free_mapping(gathered_mappings);
		} else
		{
			if(MPI_SUCCESS != MPI_Gatherv(node_local_mappings->mapping, (int)node_local_mappings->size, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, role_comm))
			{
				fprintf(stderr, "error on MPI_Gatherv for pid: %d\n", pid);
			}
		}
	}
	MPI_Bcast(new_mapping->mapping, new_mapping->size, MPI_INT, 0, MPI_COMM_WORLD);

	if(is_nodeleader)
	{
		free_mapping(node_local_mappings);
		if(pid == 0)
		{
			free(num_node_local_mappings);
			free_mapping(gathered_mappings);
		}
	}
	return MAP_SUCCESS;
}

map_status_t proc_map_node_local(const hw_topology *hw_top,
		const comm_lat *latencies,
		const graph_t *source_graph,
		const mapping_t *node_mappings,
		mapping_t *mapping)
{
	map_status_t status = MAP_SUCCESS;
	int pid, num_proc, node_number, i, j, is_nodeleader, num_node_local_mapped_vertices;
	SCOTCH_Num num_source_graph_vertices;
	SCOTCH_Num *node_local_mapped_vertices;
	arch_t *node_local_arch;
	graph_t *sub_source_graph; /* represent subset of source_graph with only node-mapped vertices */

	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	num_source_graph_vertices = 0;
	is_nodeleader = pid % hw_top->core_per_node == 0;
	if(is_nodeleader) /* this pid is responsible for its local node's process mapping */
	{
		/* generate a new source graph, being a subset of the passed sourcegraph only containing node-mapped vertices */
		node_number = pid / hw_top->core_per_node;
		SCOTCH_graphSize(source_graph, &num_source_graph_vertices, NULL);

		num_node_local_mapped_vertices = num_node_local_vertices_from_mapping(node_mappings, node_number);
		/* TODO: debugging */
		if(num_node_local_mapped_vertices > hw_top->core_per_node)
		{
			printf("pid %d got too many vertices! exiting!\n", pid);
			//return MAP_LIB_ERROR;
		}
		if(NULL != (node_local_mapped_vertices = malloc(sizeof(int) * num_node_local_mapped_vertices)))
		{
			get_node_local_vertices_from_mapping(node_mappings, node_number, node_local_mapped_vertices);
			if(NULL != (sub_source_graph = get_subgraph_from_supergraph(source_graph, num_node_local_mapped_vertices, node_local_mapped_vertices, pid)))
			{
				if(num_node_local_mapped_vertices == hw_top->core_per_node) /* All cores in the node will be utilized */
				{
					if((node_local_arch = gen_target_local(hw_top, latencies)) != NULL)
					{
#ifdef DBEUG
						printf("Performing node local process mapping... ");
#endif
						if(MAP_SUCCESS == (status = get_mapping(sub_source_graph, node_local_arch, mapping)))
						{
#ifdef DBEUG
							printf("OK\n");
#endif
							/* increase every mapped label with the local pid, so that the local mapping reflects global process ids */
							for(i = 0; i < mapping->size; i++)
							{
								mapping->mapping[i] += pid;
							}
							status = MAP_SUCCESS;
						} else
						{
#ifdef DEBUG
							printf("pid %d: failed performing mapping\n", pid);
#endif
						}
						free_arch(node_local_arch);
					} else
					{
						fprintf(stderr, "couldn't retrieve sub source graph from initial source graph\n");
						status = MAP_LIB_ERROR;
					}
				} else // All of the node's cores won't be utilized
				{
					/* TODO: remove test debug  */
					if(pid != 360)
					{
						//printf("I should not be doing this!! (pid %d)\n", pid);
					}
					if(num_node_local_mapped_vertices <= hw_top->core_per_numa)
					{
						//printf("pid %d: cores fit in numa-node!\n", pid);
						for(i = 0; i < mapping->size; i++)
						{
							//printf("pid %d: setting mapping %d to %d\n", pid, i, pid  + i);
							mapping->mapping[i] = pid + i;
						}
					} else if(num_node_local_mapped_vertices <= hw_top->core_per_cpu)
					{
						//printf("pid: %d cores fit in cpu\n", pid);
						// TODO Make a graph mapping against two leaves here!
						for(i = 0; i < mapping->size; i++)
						{
							mapping->mapping[i] = pid + i;
						}
					} else
					{
						//printf("pid: %d cores fit in node\n", pid);
						if((node_local_arch = gen_target_local(hw_top, latencies)) != NULL)
						{
#ifdef DBEUG
							printf("Performing node local process mapping... ");
#endif
							if(MAP_SUCCESS == (status = get_mapping(sub_source_graph, node_local_arch, mapping)))
							{
#ifdef DBEUG
								printf("OK\n");
#endif
								// increase every mapped label with the local pid, so that the local mapping reflects global process ids
								// Also, shift all mappings together so that the cores are used in numerical order.
								int mapped_nodes[num_node_local_mapped_vertices];
								for(i = 0; i < num_node_local_mapped_vertices; i++)
								{
									mapped_nodes[i] = 0;
								}
								for(i = 0; i < mapping->size; i++)
								{
									if(mapping->mapping[i] > num_node_local_mapped_vertices)
									{
										//printf("should swap this mapping: %d\n", mapping->mapping[i]+pid);
									} else
									{
										mapped_nodes[mapping->mapping[i]] = 1;
									}
								}
								for(i = 0; i < mapping->size; i++)
								{
									if(mapping->mapping[i] > num_node_local_mapped_vertices)
									{
										for(j = 0; j < num_node_local_mapped_vertices; j++)
										{
											if(mapped_nodes[j] == 0)
											{
												//printf("setting mapping %d to %d instead of %d (%d)\n", i, j, mapping->mapping[i], mapping->mapping[i] + pid);
												mapping->mapping[i] = j;
												mapped_nodes[j] = 1;
												j = num_node_local_mapped_vertices + 1; //break!
											}
										}
									}
								}
								for(i = 0; i < mapping->size; i++)
								{
									mapping->mapping[i] += pid;
								}
								status = MAP_SUCCESS;
							} else
							{
#ifdef DEBUG
								printf("pid %d: failed performing mapping\n", pid);
#endif
							}
							free_arch(node_local_arch);
						} else
						{
							fprintf(stderr, "couldn't retrieve sub source graph from initial source graph\n");
							status = MAP_LIB_ERROR;
						}
					}
				}
				free_graph(sub_source_graph);
			}
			free(node_local_mapped_vertices);
		}
	}
	return status;
}

map_status_t get_mapping(graph_t *source_graph, const arch_t *target_graph, mapping_t *mapping)
{
	SCOTCH_Strat *strat;
	SCOTCH_Num graph_size;
	if(mapping == NULL)
		return MAP_ARG_ERROR;
	if(mapping->mapping == NULL)
		return MAP_ARG_ERROR;

	SCOTCH_graphSize(source_graph, &graph_size, NULL);
#ifdef DEBUG
	printf("Performing mapping of graph with size: %d\n", graph_size);
#endif
	if((strat = allocate_strat()) != NULL)
	{
		if(SCOTCH_stratGraphMap(strat, STRATEGY_STRING) == 0)
		{
#ifdef DEBUG
			printf("* Starting process mapping ...\n");
#endif
			if(SCOTCH_graphMap(source_graph, target_graph, strat, mapping->mapping) == 0)
			{
#ifdef DEBUG
				printf("OK\n");
				print_mapping(res);
#endif
				free_strat(strat);
				return MAP_SUCCESS;
			} else
			{
				fprintf(stderr, "FAIL\nCould not complete process mapping.");
			}
		}
		free_strat(strat);
	}
	return MAP_LIB_ERROR;
}

map_status_t get_mapping_refinement(graph_t *source_graph, const arch_t *target_graph, mapping_t *prior_mapping, mapping_t *new_mapping)
{
	SCOTCH_Strat *strat;
	SCOTCH_Num graph_size;
	SCOTCH_Num *vmlotab;
	int i;

	if(prior_mapping == NULL)
		return MAP_ARG_ERROR;
	if(prior_mapping->mapping == NULL)
		return MAP_ARG_ERROR;

	SCOTCH_graphSize(source_graph, &graph_size, NULL);
#ifdef DEBUG
	printf("Performing mapping refinement of graph with size: %d\n", graph_size);
#endif
	if((strat = allocate_strat()) != NULL)
	{
		if(SCOTCH_stratGraphMap(strat, STRATEGY_STRING) == 0)
		{
#ifdef DEBUG
			printf("* Starting process mapping refinement...\n");
#endif
			if(NULL != (vmlotab = malloc(sizeof(SCOTCH_Num) * prior_mapping->size)))
			{
				for(i = 0; i < prior_mapping->size; i++)
				{
					vmlotab[i] = 0;
				}
				if(SCOTCH_graphRemap(source_graph, target_graph, prior_mapping->mapping, 0, vmlotab, strat, new_mapping->mapping) == 0)
				{
#ifdef DEBUG
					printf("OK\n");
					print_mapping(res);
#endif
					free_strat(strat);
					return MAP_SUCCESS;
				} else
				{
					fprintf(stderr, "FAIL\nCould not complete process mapping refinement.");
				}
				free(vmlotab);
			} else
			{
				fprintf(stderr, "FAIL\n could not allocate memory for vmlotab\n");
			}
		}
		free_strat(strat);
	}
	return MAP_LIB_ERROR;
}

void aggregate_mapping_data(mapping_t *node_mappings, int *node_offsets, mapping_t *received_mappings, mapping_t *final_mapping)
{
	int i, current_node;
	int *node_index;

	if(NULL != (node_index = malloc(sizeof(int) * node_mappings->size)))
	{
		for(i = 0; i < node_mappings->size; i++)
		{
			node_index[i] = 0;
		}
		for(i = 0; i < node_mappings->size; i++)
		{
			current_node = node_mappings->mapping[i];
			final_mapping->mapping[i] = received_mappings->mapping[node_offsets[current_node] + node_index[current_node]];
			node_index[current_node]++;
		}
		free(node_index);
	} else
	{
		fprintf(stderr, "couldn't allocate memory for node indexes!\n");
	}
}

graph_t* make_arch_graph(const int ***deco_matrix, const hw_topology *hw_top)
{
	int i, j, current_edge;
	int num_nodes = (int) ceil((double)hw_top->reserved_cores / hw_top->core_per_node);
	graph_t *target_graph = NULL;
	SCOTCH_Num vertnbr, edgenbr;
	SCOTCH_Num *vendtab, *vlbltab, *velotab, *verttab, *edgetab, *edlotab;

	if((target_graph = allocate_graph()) != NULL)
	{
		vertnbr = num_nodes;
		edgenbr = vertnbr * (vertnbr - 1);
		if((velotab = malloc(sizeof(SCOTCH_Num) * vertnbr)) != NULL)
		{
			if((vlbltab = malloc(sizeof(SCOTCH_Num) * vertnbr)) != NULL)
			{
				if((verttab = malloc(sizeof(SCOTCH_Num) * (vertnbr + 1))) != NULL)
				{
					if((edgetab = malloc(sizeof(SCOTCH_Num) * edgenbr)) != NULL)
					{
						if((edlotab = malloc(sizeof(SCOTCH_Num) * edgenbr)) != NULL)
						{
							vendtab = &verttab[1];
							for(i = 0; i < vertnbr; i++)
							{
								if(i < (num_nodes - 1))
								{
									velotab[i] = hw_top->core_per_node;
								} else
								{
									if(0 == (hw_top->reserved_cores % hw_top->core_per_node))
									{
										velotab[i] = hw_top->core_per_node;

									} else
									{
										velotab[i] = hw_top->reserved_cores % hw_top->core_per_node;
									}
								}
								verttab[i] = i * (vertnbr - 1);
								vlbltab[i] = i;
							}
							verttab[vertnbr] = edgenbr;
							current_edge = 0;
							for(i = 0; i < vertnbr; i++)
							{
								for(j = 0; j < vertnbr; j++)
								{
									if(i != j)
									{
										edgetab[current_edge] = j;
										edlotab[current_edge] = get_deco_matrix_val(i, j, deco_matrix);
										current_edge++;
									}
								}
							}
#ifdef DEBUG
							printf("Creating target graph based on decomposition matrix...\n");
#endif
							if(SCOTCH_graphBuild(target_graph,
										0,
										vertnbr,
										verttab,
										vendtab,
										velotab,
										vlbltab,
										edgenbr,
										edgetab,
										edlotab) == 0)
							{
#ifdef DEBUG
								printf("Success!\n");
#endif
								/*
								FILE *fp;
								fp = fopen("arch.grf", "w");
								SCOTCH_graphSave(target_graph, fp);
								fclose(fp);
								*/
								return target_graph;
							}
							free(edlotab);
						}
						free(edgetab);
					}
					free(verttab);
				}
				free(vlbltab);
			}
			free(velotab);
		}
		free_graph(target_graph);
	}
	return NULL;
}

arch_t* make_arch(const int ***deco_matrix, const hw_topology *hw_top)
{
	arch_t *target_arch;
	graph_t *target_graph;
	SCOTCH_Strat *strat;

#ifdef DEBUG
	int i,j;
	int num_nodes = (int) ceil((double)(hw_top->reserved_cores / hw_top->core_per_node));
	for(i = 0; i < num_nodes - 1; i++)
	{
		for(j = 0; j <= i; j++)
		{
			printf("deco_matrix[%d][%d]: %d\n", i, j, (*deco_matrix)[i][j]);
		}
	}
#endif
	if((target_graph = make_arch_graph(deco_matrix, hw_top)) != NULL)
	{
		if((target_arch = allocate_arch()) != NULL)
		{
			if((strat = allocate_strat()) != NULL)
			{
				if(SCOTCH_stratGraphBipart(strat, STRATEGY_BIPARTITIONING) == 0)
				{
#ifdef DEBUG
					printf("structures allocated, creating arch\n");
#endif
					if(SCOTCH_archBuild(target_arch,
								target_graph,
								0,
								NULL,
								strat) == 0)
					{
#ifdef DEBUG
						printf("Success!\n");
#endif
						free_strat(strat);
						free_graph(target_graph);
						return target_arch;
					}
				}
				free_strat(strat);
			}
			free_arch(target_arch);
		}
	}
	return NULL;
}

/*
 * gen_target_all_to_all
 * Generates a target graph based on an all-to-all measurement.
 */
arch_t* gen_target_all_to_all(const hw_topology *hw_top, int pid)
{
	int **deco_matrix;
	int i, j, num_nodes, nid, role_key, is_node_leader, role_pid;
	int nodes_to_jump, elements_in_diagonal;
	int time_measurement;
	arch_t *target_graph = NULL;
	MPI_Comm role_comm;
	MPI_Status status;

	nid = pid / hw_top->core_per_node;
	is_node_leader = (pid % hw_top->core_per_node) == 0;
	if((pid % hw_top->core_per_node) == 0)
	{
		role_key = nid;
	} else
	{
		role_key = pid - (pid * (pid / hw_top->core_per_node));
	}
	MPI_Comm_split(MPI_COMM_WORLD, is_node_leader, role_key, &role_comm);
	MPI_Comm_rank(role_comm, &role_pid);

	if(is_node_leader)
	{
		num_nodes = (int) ceil((double)hw_top->reserved_cores / hw_top->core_per_node);
		if(pid == 0)
		{
			allocate_deco_matrix(num_nodes, &deco_matrix);
		}
		nodes_to_jump = 1;
		for(i = 0; i < num_nodes - 1; i++)
		{
			elements_in_diagonal = num_nodes - i - 1;
			/* diagonal gotten from iterating over j and with "f(j-i, j)" */
			if(elements_in_diagonal > nodes_to_jump) /* diagonal contains atleast two elements sharing the same node, thus not all measurements can be done at once (but in two rounds) */
			{
				for(j = i; j < num_nodes - 1; j++)
				{
					if(role_pid == j - i)
					{
						time_measurement = (int) (send_ping(hw_top->core_per_node * (j + 1), MEASUREMENT_ITERATIONS) * MEASUREMENT_CLOCK_MULTIPLIER);
						if(role_pid != 0)
						{
							MPI_Send(&time_measurement, 1, MPI_INT, 0, MPI_TAG, role_comm);
						} else
						{
							set_deco_matrix_val(j - i, j + 1, time_measurement, &deco_matrix);
						}
					} else if(role_pid == j + 1 )
					{
						respond_to_ping(hw_top->core_per_node * (j - i), MEASUREMENT_ITERATIONS);
					} else if(role_pid == 0)
					{
						MPI_Recv(&time_measurement, 1, MPI_INT, j - i, MPI_TAG, role_comm, &status);
						set_deco_matrix_val(j - i, j + 1, time_measurement, &deco_matrix);
					}
					/* if iterated over 'incrementor' nodes, jump over 'incremetor' nodes */
					if((j - i + 1) % nodes_to_jump == 0 )
					{
						j += nodes_to_jump;
					}
				}
				MPI_Barrier(role_comm);
				for(j = i + nodes_to_jump; j < num_nodes - 1; j++)
				{
					if(role_pid == j - i)
					{
						time_measurement = (int) (send_ping(hw_top->core_per_node * (j + 1), MEASUREMENT_ITERATIONS) * MEASUREMENT_CLOCK_MULTIPLIER);
						if(role_pid != 0)
						{
							MPI_Send(&time_measurement, 1, MPI_INT, 0, MPI_TAG, role_comm);
						} else
						{
							set_deco_matrix_val(j - i, j + 1, time_measurement, &deco_matrix);
						}
					} else if(role_pid == j + 1)
					{
						respond_to_ping(hw_top->core_per_node * (j - i), MEASUREMENT_ITERATIONS);
					} else if(role_pid == 0)
					{
						MPI_Recv(&time_measurement, 1, MPI_INT, j - i, MPI_TAG, role_comm, &status);
						set_deco_matrix_val(j - i, j + 1, time_measurement, &deco_matrix);
					}
					/* if iterated over 'incrementor' nodes, jump over 'incremetor' nodes */
					if((j - i  - nodes_to_jump + 1) % nodes_to_jump == 0 )
					{
						j += nodes_to_jump;
					}
				}
			} else /* diagonal contains no two elements sharing the same node, thus all measurements can be done at once */
			{
				for(j = i; j < num_nodes - 1; j++)
				{
					if(role_pid == j - i)
					{
						time_measurement = (int) (send_ping(hw_top->core_per_node * (j + 1), MEASUREMENT_ITERATIONS) * MEASUREMENT_CLOCK_MULTIPLIER);
						if(role_pid != 0)
						{
							MPI_Send(&time_measurement, 1, MPI_INT, 0, MPI_TAG, role_comm);
						} else
						{
							set_deco_matrix_val(j - i, j + 1, time_measurement, &deco_matrix);
						}
					} else if(role_pid == j + 1)
					{
						respond_to_ping(hw_top->core_per_node * (j - i), MEASUREMENT_ITERATIONS);
					} else if(role_pid == 0)
					{
						MPI_Recv(&time_measurement, 1, MPI_INT, j - i, MPI_TAG, role_comm, &status);
						set_deco_matrix_val(j - i, j + 1, time_measurement, &deco_matrix);
					}
				}
			}
			nodes_to_jump++;
		}

		if(pid == 0)
		{
			target_graph = make_arch((const int***)&deco_matrix, hw_top);
			free_deco_matrix(num_nodes, &deco_matrix);
		}
	}
	return target_graph;
}

/*
 * gen_target_manhattan
 * Generates a target graph based on a distance function defined from prior measurements.
 *
 */
arch_t* gen_target_manhattan(const hw_topology *hw_top, int pid, const comm_lat *latencies)
{
	/* Build deco_matrix */
	int num_nodes;
	int **deco_matrix;
	int i, j;
	arch_t *target_graph = NULL;
	mesh_coord_t n1_pos, n2_pos;
	num_nodes = (int)ceil((double)hw_top->reserved_cores / hw_top->core_per_node);
	//printf("num nodes: %d\n", num_nodes);

	if(pid == 0)
	{
		allocate_deco_matrix(num_nodes, &deco_matrix);
		for(i = 1; i < num_nodes ; i++)
		{
			for(j = 0; j < i; j++)
			{
				get_pos_from_pid(i * hw_top->core_per_node, &n1_pos);
				get_pos_from_pid(j * hw_top->core_per_node, &n2_pos);
				//printf("from %d to %d, distance: %d\n", n1_nid, n2_nid, latencies->inter_node_fix + (latencies->inter_node_hop * get_torus_mesh_manhattan_distance(&(hw_top->max_dim), &n1_pos, &n2_pos)));
				set_deco_matrix_val(i, j, latencies->inter_node_fix + (latencies->inter_node_hop * get_torus_mesh_manhattan_distance(&(hw_top->max_dim), &n1_pos, &n2_pos)), &deco_matrix);
			}
		}
		target_graph = make_arch((const int***) &deco_matrix, hw_top);
		free_deco_matrix(num_nodes, &deco_matrix);
	}
	return target_graph;
}

/*
 * gen_target_local
 * Generates an intra-node target graph based on core layout and comm_lat measurements.
 */
arch_t* gen_target_local(const hw_topology *hw_top, const comm_lat *latencies)
{
	arch_t *arch;
	SCOTCH_Num levelnumber = 3;
	SCOTCH_Num sizetab[3] = {hw_top->cpu_per_node, hw_top->numa_per_cpu, hw_top->core_per_numa};
	SCOTCH_Num linktab[3] = {latencies->intra_node, latencies->intra_cpu, latencies->intra_numa};


	if((arch = allocate_arch()) != NULL)
	{
		if(0 != SCOTCH_archTleaf(arch,
					levelnumber,
					sizetab,
					linktab))
		{
			free_arch(arch);
		}
	}
	if(arch == NULL)
	{
		fprintf(stderr, "Couldn't create tleaf graph\n");
	}
	return arch;
}

/*
 * gen_comm_lat
 * Generates a comm_lat struct from measurements in a reserved set of nodes by least square fitting the measured data
 * to a first-degree polynomial
 */
map_status_t gen_comm_lat(const hw_topology *hw_top, comm_lat *latencies)
{
	/*
	   int pid;
	   map_status_t result;
	   MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	   if((result = measure_intra_node_distances(hw_top, latencies)) != MAP_SUCCESS)
	   {
	   fprintf(stderr, "problem measuring intra-node distance");
	   return result;
	   }
	 */
	return measure_manhattan_distance(hw_top, latencies);
}

/*
 * print_mapping
 */
void print_mapping(const SCOTCH_Num *mappings, int num_vertices)
{
	int i;
	for(i=0;i<num_vertices;i++){
		printf("%d\t%d\n", i, mappings[i]);
	}
}

int graph_size(const graph_t *graph)
{
	int size;
	SCOTCH_graphSize(graph, &size, NULL);
	return size;
}

/*
 * allocate_deco_matrix
 */
void allocate_deco_matrix(int num_nodes, int ***deco_matrix)
{
	int i;
	/* pointer to row-pointers. */
	*deco_matrix = malloc(sizeof(int*) * num_nodes);
	for(i = 0; i < num_nodes - 1; i++)
	{
		/* Row pointer */
#ifdef DEBUG
		printf("allocating row %d of size: %d\n", i,  i + 1);
#endif
		(*deco_matrix)[i] = malloc(sizeof(int) * (i + 1));
	}
}

/*
 * set_deco_matrix_val
 *
 */
void set_deco_matrix_val(int i, int j, int val, int ***deco_matrix)
{
	i > j ? ((*deco_matrix)[i - 1][j] = val) : ((*deco_matrix)[j - 1][i] = val);
}

/*
 * get_deco_matrix_val
 */
int get_deco_matrix_val(int i, int j, const int ***deco_matrix)
{
	return i > j ? (*deco_matrix)[i - 1][j] : (*deco_matrix)[j - 1][i];
}

/*
 * free_deco_matrix
 *
 */
void free_deco_matrix(int num_nodes, int ***deco_matrix)
{
	int i;
	for(i = 0; i < num_nodes - 1; i++)
	{
		free((*deco_matrix)[i]);
	}
	free(*deco_matrix);
}

arch_t* allocate_arch()
{
	arch_t *arch;
	if((arch = SCOTCH_archAlloc()) != NULL)
	{
		if(SCOTCH_archInit(arch) == 0)
		{
			return arch;
		}
		free(arch);
	}
	return NULL;
}

SCOTCH_Strat* allocate_strat()
{
	SCOTCH_Strat *strat;
	if((strat = SCOTCH_stratAlloc()) != NULL)
	{
		if(SCOTCH_stratInit(strat) == 0)
		{
			return strat;
		}
		free(strat);
	}
	return NULL;
}

graph_t* allocate_graph()
{
	graph_t *graph;
	if((graph = SCOTCH_graphAlloc()) != NULL)
	{
		if(SCOTCH_graphInit(graph) == 0)
		{
			return graph;
		}
		free(graph);
	}
	return NULL;
}

mapping_t* allocate_mapping(size_t size)
{
	mapping_t *map;
	if((map = malloc(sizeof(mapping_t))) != NULL)
	{
		if((map->mapping = malloc(sizeof(SCOTCH_Num) * size)) != NULL)
		{
			map->size = size;
			return map;
		}
		free(map);
	}
	return NULL;
}

graph_t* load_graph(const char *file_name)
{
	graph_t *graph;
	FILE *fp;
	if((graph = allocate_graph()) != NULL)
	{
		if((fp = fopen(file_name, "r")) != NULL)
		{
			if(SCOTCH_graphLoad(graph, fp, -1, 0) == 0)
			{
				fclose(fp);
				return graph;
			}
			fclose(fp);
		}
		free_graph(graph);
	}
	return NULL;
}

arch_t* load_arch(const char *file_name)
{
	arch_t *arch;
	FILE *fp;
	if((arch = allocate_arch()) != NULL)
	{
		if((fp = fopen(file_name, "r")) != NULL)
		{
			if(SCOTCH_archLoad(arch, fp) == 0)
			{
				fclose(fp);
				return arch;
			}
			fclose(fp);
		}
		free_arch(arch);
	}
	return NULL;
}

int load_mapping(const char *file_name, mapping_t *mapping)
{
	char *separator = " ";
	char *line;
	size_t line_len = 40;
	FILE *fp;
	int index, value;

	if(NULL != (line = malloc(line_len)))
	{
		if((fp = fopen(file_name, "r")) != NULL)
		{
			while(-1 != getline(&line, &line_len, fp))
			{
				index = atoi(strtok(line, separator));
				value = atoi(strtok(NULL, separator));
				mapping->mapping[index] = value;
			}
			fclose(fp);
			free(line);
			return 0;
		}
		free(line);
	}
	return -1;
}

void free_arch(arch_t *arch)
{
	if(arch != NULL)
	{
		SCOTCH_archExit(arch);
		free(arch);
		arch = NULL;
	}
}

void free_strat(SCOTCH_Strat *strat)
{
	if(strat != NULL)
	{
		SCOTCH_stratExit(strat);
		free(strat);
		strat = NULL;
	}
}

void free_graph(graph_t *graph)
{
	if(graph != NULL)
	{
		SCOTCH_graphExit(graph);
		free(graph);
		graph = NULL;
	}
}

void free_mapping(mapping_t *map)
{
	if(map->mapping != NULL)
	{
		free(map->mapping);
		map->mapping = NULL;
	}
}

