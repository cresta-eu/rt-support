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
 * graph.h
 * Headerfile for the Graph API.
 * Supplies the funcionality of generating a target graph file (a decomposition
 * matrix) from a set of process id:s, a hardware architecture (at this time
 * assuming something similar to Cray XE6) and measured latencies in the
 * network.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "topology.h"

/*
 * get_array_size
 * Returns the number of entries (size) needed to make a decomposition matrix
 * and store it in an array. A decomposition matrix is a lower/upper-triangular
 * matrix that gives the bidirectional distance between two nodes in a graph
 *
 * Params: int, the number of processes. Expressed as a full matrix, this would
 * 			require a N*N matrix
 * Returns: int, the number of entries (the size of the array) needed to express
 * 			a decomposition matrix for the passed number of processes.
*/
int get_array_size(int pids);
/*
 * set_distance_between_pids
 * Updates the array of process distances with a distance between two pids
 *
 * Params: int*, the array of distances. This array is one dimensional and not a
 * 			two dimensional N*N matrix as one might expect. The reason for this is that
 * 			more than a half of such a matrix would be unused as the decomposition matrix
 * 			is lower triangular.
 * 		   int, the id of the first process
 *		   int, the id of the second proces
 *		   int, the distance between the two (typically gotten from a
 *		  	 measurement)
 * Returns: void
 */
void set_distance_between_pids(int* pids, int num_pids, int pid_1, int pid_2);
int get_core_offset(int pid, int core_per_node);
int vertex_is_in_list(int vertex, size_t list_size, const int *vertex_list);
void set_node_local_distances(comm_lat *latncies, int* distances, int start_index, int core_offset, const hw_topology* hw_top);
void set_node_external_distances(comm_lat *latencies, int* distances, int start_index, int nid_1, int nid_2,  const hw_topology* hw_top);
int num_node_local_vertices_from_mapping(const mapping_t *mapping, int node_number);
void get_node_local_vertices_from_mapping(const mapping_t *mapping, int node_number, int *res);
graph_t* get_subgraph_from_supergraph(const graph_t *super_graph, size_t vertex_list_size, const int *vertex_list, int pid);
void set_subgraph_from_supergraph(SCOTCH_Num **sub_vertlbl, SCOTCH_Num **sub_vendtab, SCOTCH_Num **sub_verttab, SCOTCH_Num **sub_edgetab, SCOTCH_Num **sub_edlotab, SCOTCH_Num **sub_velotab, const SCOTCH_Num *super_verttab, const SCOTCH_Num *super_edgetab, const SCOTCH_Num *super_edlotab, const SCOTCH_Num *super_velotab, int num_subgraph_vertices, const int *subgraph_vertex_list, int num_vertices, int pid);
SCOTCH_Num get_num_subgraphedges_from_supergraph(const SCOTCH_Num *super_verttab, const SCOTCH_Num *super_edgetab, int num_subgraph_vertices, const int *subgraph_vertex_list);
#endif

