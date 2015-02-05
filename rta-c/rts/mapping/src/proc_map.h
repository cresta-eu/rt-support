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

#ifndef PROCMAP_H
#define PROCMAP_H

#include <scotch.h>
#include <stdio.h>
#include <string.h>
#include "proc_map_types.h"
#include "topology.h"


/*
 * proc_map
 * The process mapper function. Main function of the project, who's task is to create a process mapping using scotch.
 * For mapping refinement, use proc_map_refine.
*/
map_status_t proc_map(const hw_topology *hw_top,
						const comm_lat *latencies,
						graph_t *source_graph,
						arch_t *target_graph,
						mapping_t *mapping);


map_status_t proc_map_refine(const hw_topology *hw_top,
								const comm_lat *latencies,
								graph_t *source_graph,
								arch_t *target_graph,
								mapping_t *prior_mapping,
								mapping_t *new_mapping);
/*
 * gen_target_local
 * Generates an intra-node target graph based on core layout and comm_lat measurements.
 *
*/
arch_t* gen_target_local(const hw_topology *hw_top, const comm_lat *latencies);
/*
 * gen_comm_lat
 * Generates a comm_lat struct from measurements in a reserved set of nodes by least square fitting the measured data
 * to a first-degree polynomial
*/
map_status_t gen_comm_lat(const hw_topology *hw_top, comm_lat *latencies);
/*
 * gen_target_manhattan
 * Generates a target graph based on a distance function defined from prior measurements.
 *
*/
arch_t* gen_target_manhattan(const hw_topology *hw_top, int pid, const comm_lat *latencies);

arch_t* gen_target_all_to_all(const hw_topology *hw_top, int pid);

map_status_t get_mapping(graph_t *source_graph, const arch_t *target_graph, mapping_t *mapping);
map_status_t get_mapping_refinement(graph_t *source_graph, const arch_t *target_graph, mapping_t *prior_mapping, mapping_t *new_mapping);

void aggregate_mapping_data(mapping_t *node_mappings, int *node_offsets, mapping_t *received_mappings, mapping_t *final_mapping);

/*
* proc_map_node_local
* performs intra-node mapping
*/
map_status_t proc_map_node_local(const hw_topology *hw_top,
							const comm_lat *latencies,
							const graph_t *source_graph,
							const mapping_t *node_mappings,
							mapping_t *mapping);


arch_t* make_arch(const int ***deco_matrix, const hw_topology *hw_top);
graph_t* make_arch_graph(const int ***deco_matrix, const hw_topology *hw_top);

/*
 * print_mapping
 * Prints the mapped values on format: <source vertex id>		<mapped target id><CR>
*/
void print_mapping(const SCOTCH_Num *mappings, int num_vertices);

int graph_size(const graph_t *graph);

graph_t* load_graph(const char *file_name);
arch_t* load_arch(const char *file_name);
int load_mapping(const char *file_name, mapping_t *mapping);

void allocate_deco_matrix(int num_nodes, int ***deco_matrix);
mapping_t* allocate_mapping(size_t size);
arch_t* allocate_arch();
graph_t* allocate_graph();
SCOTCH_Strat* allocate_strat();

void free_deco_matrix(int num_nodes, int ***deco_matrix);
void free_arch(arch_t *arch);
void free_graph(graph_t *graph);
void free_strat(SCOTCH_Strat *strat);
void free_mapping(mapping_t *mapping);

/*
 * void set_deco_matrix_val
*/
void set_deco_matrix_val(int i, int j, int val, int ***deco_matrix);

/*
 * void set_deco_matrix_val
*/
int get_deco_matrix_val(int i, int j, const int ***deco_matrix);

#endif

