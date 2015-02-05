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

#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include "topology.h"
#include "proc_map_types.h"

/* Performs node-local distance measuring on the first reserved core and updates hw_top->latencies accordingly */
map_status_t measure_intra_node_distances(const hw_topology *hw_top, comm_lat *latencies);

/*
 * find_connected_nodes
 * looks for two nodes with the same position in the interconnect, thus sharing gemini-node
 * updates pid_1 and pid_2 with the first pids of the two, if found, else -1 as their values.
 */
void find_connected_nodes(const hw_topology *hw_top, int num_ext_nodes, int *pid_1, int *pid_2);

/*
 * measure_manhattan_distance
 * updates hw_top->latencies with latencies taken from manhattan-distance measurements.
 */
map_status_t measure_manhattan_distance(const hw_topology *hw_top, comm_lat *latencies);

/* Updates the passed hw_topology's comm_lat with node-internal distances based on passed measurements in *local_time_res */
void set_manhattan_intra_node_distances(const hw_topology *hw_top, comm_lat *latencies, double *local_time_res);

/* Updates the passed hw_topology's comm_lat with node-external distances (fix+hop) based on passed measurements in *ext_time_res */
map_status_t set_manhattan_inter_node_distances(int *fix, int *hop, int *ext_time_res, int *num_hops, int num_ext_nodes);

/* Expects iteration amount of messages and acknowledges them all.
	Receive and send operations are blocking. */
void respond_to_ping(int pid, int iterations);

/* Sends <iteration> amount of messages to the supplied pid and expects an acknowledgement for every message
	sent. */
double send_ping(int pid, int iterations);

#endif

