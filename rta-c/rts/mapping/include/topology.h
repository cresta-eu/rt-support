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
 * topology.h
 * Headerfile for the Topology API.
 * A facade for hardware topology-calls in Cray XE6-systems such as KTH/PDC's
 * Lindgren, to be used in process mapping programs and other programs relying
 * on process location in the cluster.
 *
 * Author: Sebastian tunstig, tunstig@kth.se
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "proc_map_types.h"

#define TOP_SUCC 0
#define TOP_FAIL -1

/* strategy specifiers */
#define T_MANHATTAN 1
#define T_CLUSTERING 2

/*
 * get_nid_from_pid
 * Used to retreive a process's node id. Function can return the node id of any
 * process that is a part of the running mpi-program.
 *
 * Param: int, the id of the process
 * 		  int*, pointer to an integer where result (node id) is stored.
 * Return value: Returns TOP_SUCC uppon success and TOP_FAIL if the operation
 * 				couldn't complete.
 */
int get_nid_from_pid(int pid, int* nid);
/*
 * get_pos_from_nid
 * Used to retreive a node's physical location (in coordinates) in the the
 * cluster. Node position is based on the NIC's location in the network topology
 * and hence every position is shared by two nodes.
 *
 * Param: int, the id of the node
 * 		  mesh_coord_t*, pointer to an mesh_coord_t where the position
 * 		  	is stored.
 * Return value: Returns TOP_SUCC uppon success and TOP_FAIL if the operation
 * 				couldn't complete.
 */
int get_pos_from_nid(int nid, mesh_coord_t* pos);

/*
 * get_pos_from_pid
 * Used to retreive a node's physical location (in coordinates) in the the
 * cluster. Node position is based on the NIC's location in the network topology
 * and hence every position is shared by two nodes.
 *
 * Param: int, the id of the process
 * 		  mesh_coord_t*, pointer to an mesh_coord_t where the position
 * 		  	is stored.
 * Return value: Returns TOP_SUCC uppon success and TOP_FAIL if the operation
 * 				couldn't complete.
 */
int get_pos_from_pid(int pid, mesh_coord_t* pos);

/*
 * get_topo_max_dim
 * Used to retreive the greatest coordinate in every dimension of the system's
 * network topology. This is due to the cyclic property of a 3D Torus, when
 * calculating the number of routing hops in one dimension from one point to
 * another, there's always two possible ways of routing the traffic, and in most
 * cases one is shorter than the other. This function is thus used when finding
 * shortest paths in the network.
 *
 * Param: mesh_coord_t*, pointer to an mesh_coord_t struct, where the maximum
 * values of every dimension (x, y, z) is stored upon success.
 * Return value: Returns TOP_SUCC uppon success and TOP_FAIL if the operation
 * 				couldn't complete.
 */
int get_topo_max_dim(mesh_coord_t *mxyz);

/*
 * get_torus_mesh_manhattan_distance
 * Used to retreive the distance between two coordinates in the network.
 * The result is the shortest manhattan distance, and is the expected actual
 * distance. Cray provides no routing information today so this can only be
 * estimated or concluded from measurements.
 *
 * Param: mesh_coord_t, the maximum coordinates for all dimensions in the
 * 			network.
 * 		  mesh_coord_t, coordinates of the first point
 *		  mesh_coord_t, coordinates of the other point
 * Return value: Returns an integer, representing the shortest distance in the
 * 				network between the points.
 */
int get_torus_mesh_manhattan_distance(const mesh_coord_t* max_network, const mesh_coord_t *p1, const mesh_coord_t *p2);


/*
 * struct comm_lat
 * Used to store measured latencies between cores in a XE6 cluster.
 * Usage: Run measurements on all cores of atleast three nodes once (either by yourself or with the supplied program
 * measurements.c). Store the average results in the struct and use it to create a target graph (representing the
 * network topology) as close to the real system as possible.
 */
typedef struct {
	int intra_numa; /* latency between two cores that share NUMA-node */
	int intra_cpu; /* latency between two cores that share cpu but not NUMA-node */
	int intra_node; /* latency between two cores that share node but not cpu */
	int inter_node_fix; /* latency between two cores within the same gemini-node */
	int inter_node_hop; /* The additional latency for communication over the interconnect (for each routing hop) */
} comm_lat;

/*
 * struct hw_topology
 * Used to define the hardware topology. Assuming a similar hardware model as
 * the Cray XE6. Struct enables similar systems to take usage of the API.
 *
 */
typedef struct {
	int core_per_numa;
	int numa_per_cpu;
	int cpu_per_node;
	int core_per_cpu;
	int core_per_node; /*can be obtained from previous variables, but is used for faster search */
	int node_per_nic;
	int reserved_cores; /* The number of reserved cores in system for this execution */
	mesh_coord_t max_dim;
} hw_topology;

/*
 * get_inter_node_dist
 * Returns the estimated latencies between two cores placed on two different nodes, based on
 * 		measurements stored in a comm_dist_crayxe6 strcut.
 * Param: comm_dist_crayxe6*, pointer to the struct containing all measured data.
 * 		  int, the first node id.
 *		  int, the second node id.
 * Return value: the estimated latency between cores of 'hops' distance between their nodes.
 */
int get_inter_node_lat(const comm_lat* this, const mesh_coord_t* max_coord, int nid_1, int nid_2);

/*
 * mesh_coord_t_equals
 * Compares two mesh_coord_t structs and returns a non-zero value if they
 * have the same coordinates.
 *
 */
int mesh_coord_t_equals(const mesh_coord_t *c1, const mesh_coord_t *c2);

/*
 * check_topology
 * checks that the passed hw_topology struct's members have valid values.
 * returns 0 on failure and !0 on success.
 */
int check_topology(const hw_topology *hw_top);
#endif /*TOPOLOGY_H*/

