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
 * topology.c
 * API for retreiving system information regarding process to node-mappings and
 * distance between nodes.
 * System calls are specific for Cray XE6-systems and the implementation is
 * tested on KTH/PDC - Lindgren.
 * The API is a facade-like middleware that uses RCA and PMI.
 * For information about API-calls, see headerfile topology.h
 *
 * Author: Sebastian Tunstig, tunstig@kth.se
*/

#include <stdio.h>
#include <stdlib.h>

#if CMK_CRAYXE
#include <pmi.h>
#include <rca_lib.h>
#endif

#include "proc_map_types.h"
#include "topology.h"


int get_nid_from_pid(int pid,int* nid)
{
#if CMK_CRAYXE

	int rc;
	PMI_BOOL initialized;
	PMI_Initialized(&initialized);
	if (initialized!=PMI_TRUE)
	{
		int spawned;
		rc = PMI_Init(&spawned);
		if (rc!=PMI_SUCCESS)
		{
			PMI_Abort(rc,"PMI_Init failed");
			fprintf(stderr, "PMI_Init(&int) failed\n");
			return TOP_FAIL;
		}
	}

	rc = PMI_Get_nid(pid, nid);
	if (rc!=PMI_SUCCESS)
	{
		PMI_Abort(rc,"PMI_Get_nid failed");
		fprintf(stderr, "PMI_Get_nid(int, &int) failed\n");
		return TOP_FAIL;
	}
#endif
	return TOP_SUCC;
}

int get_pos_from_nid(int nid, mesh_coord_t* pos)
{
#if CMK_CRAYXE

	#ifdef DEBUG
	printf("*getting position for nid: %i\n", nid);
	#endif
	PMI_BOOL initialized;
	PMI_Initialized(&initialized);
	if (initialized != PMI_TRUE)
	{
		int spawned;
		int rc = PMI_Init(&spawned);
		if (rc != PMI_SUCCESS)
		{
			PMI_Abort(rc,"PMI_Init failed");
			return TOP_FAIL;
		}
	}
	//rca_get_meshcoord( (uint16_t) nid, pos);
	rca_get_meshcoord( (uint16_t)nid, pos);
	#ifdef DEBUG
	printf("*Result: (%i,%i,%i)\n", pos->mesh_x, pos->mesh_y, pos->mesh_z);
	#endif
#endif

	return TOP_SUCC;
}

int get_pos_from_pid(int pid, mesh_coord_t* pos)
{
#if CMK_CRAYXE
	int nid;
	#ifdef DEBUG
	printf("*getting position for pid: %i\n", pid);
	#endif
	PMI_BOOL initialized;
	PMI_Initialized(&initialized);
	if (initialized != PMI_TRUE)
	{
		int spawned;
		int rc = PMI_Init(&spawned);
		if (rc != PMI_SUCCESS)
		{
			PMI_Abort(rc,"PMI_Init failed");
			return TOP_FAIL;
		}
	}
	if(TOP_FAIL == get_nid_from_pid(pid, &nid))
	{
		fprintf(stderr, "couldn't retreive nid from pid\n");
		return TOP_FAIL;
	}
	rca_get_meshcoord( (uint16_t)nid, pos);
	#ifdef DEBUG
	printf("*Result: (%i,%i,%i)\n", pos->mesh_x, pos->mesh_y, pos->mesh_z);
	#endif
#endif

	return TOP_SUCC;
}

int get_topo_max_dim(mesh_coord_t *coord)
{
#if CMK_CRAYXE
	rca_get_max_dimension(coord);
#endif
	return TOP_SUCC;
}

/*
 * TODO: Inform that arg 2 & 3 are not passed const and are possibly changed during execution (members values swapped).
 */
int get_torus_mesh_manhattan_distance(const mesh_coord_t* max_network, const mesh_coord_t *p1, const mesh_coord_t *p2)
{
	/*
	printf("Comparing (%i,%i,%i) with (%i,%i,%i)\n", p1->mesh_x, p1->mesh_y, p1->mesh_z, p2->mesh_x, p2->mesh_y, p2->mesh_z);
	printf("Maximum for dimensions: (%i,%i,%i)\n", max_network->mesh_x, max_network->mesh_y, max_network->mesh_z);
	*/
	int distance = 0;

#if CMK_CRAYXE

	mesh_coord_t pos_1, pos_2;
	if(p2->mesh_x != p1->mesh_x)
	{
		if(p1->mesh_x > p2->mesh_x){
			pos_2.mesh_x = p1->mesh_x;
			pos_1.mesh_x = p2->mesh_x;
		} else
		{
			pos_1.mesh_x = p1->mesh_x;
			pos_2.mesh_x = p2->mesh_x;
		}

		if((pos_2.mesh_x - pos_1.mesh_x ) < (pos_1.mesh_x + max_network->mesh_x - pos_2.mesh_x))
		{
			distance += (pos_2.mesh_x - pos_1.mesh_x);
		} else
		{
			distance += (pos_1.mesh_x + max_network->mesh_x - pos_2.mesh_x);
		}
	}

	if(p2->mesh_y != p1->mesh_y)
	{
		if(p1->mesh_y > p2->mesh_y){
			pos_2.mesh_y = p1->mesh_y;
			pos_1.mesh_y = p2->mesh_y;
		} else
		{
			pos_1.mesh_y = p1->mesh_y;
			pos_2.mesh_y = p2->mesh_y;
		}
		if((pos_2.mesh_y - pos_1.mesh_y ) < (pos_1.mesh_y + max_network->mesh_y - pos_2.mesh_y))
		{
			distance += (pos_2.mesh_y - pos_1.mesh_y);
		} else
		{
			distance += (pos_1.mesh_y + max_network->mesh_y - pos_2.mesh_y);
		}
	}

	if(p2->mesh_z != p1->mesh_z)
	{
		if(p1->mesh_z > p2->mesh_z){
			pos_2.mesh_z = p1->mesh_z;
			pos_1.mesh_z = p2->mesh_z;
		} else
		{
			pos_2.mesh_z = p2->mesh_z;
			pos_1.mesh_z = p1->mesh_z;
		}
		if((pos_2.mesh_z - pos_1.mesh_z ) < (pos_1.mesh_z + max_network->mesh_z - pos_2.mesh_z))
		{
			distance += (pos_2.mesh_z - pos_1.mesh_z);
		} else
		{
			distance += (pos_1.mesh_z + max_network->mesh_z - pos_2.mesh_z);
		}
	}
	//printf("calculated distance: %d\n", distance);
#endif

	return distance;
}

int get_inter_node_lat(const comm_lat* this, const mesh_coord_t* max_coord, int nid_1, int nid_2)
{
	mesh_coord_t nid_1_pos;
	mesh_coord_t nid_2_pos;
	if((TOP_FAIL == get_pos_from_nid(nid_1, &nid_1_pos)) || (TOP_FAIL == get_pos_from_nid(nid_2, &nid_2_pos)))
	{
		fprintf(stderr, "Problem with get_pos_from_nid / rca_mesH-coord_t_equals\n");
		return -1;
	}
	if(mesh_coord_t_equals(&nid_1_pos, &nid_2_pos))
	{
		return this->inter_node_fix;
	}
	int hops = get_torus_mesh_manhattan_distance(max_coord, &nid_1_pos, &nid_2_pos);
	return this->inter_node_fix + (hops * this->inter_node_hop);
}

/*
 * mesh_coord_t_equals
 * returns a non-zero value if the passed coordinate-structs are "equal" (i.e.
 * x/y/z-coordinates are the same)
 */
int mesh_coord_t_equals(const mesh_coord_t *c1, const mesh_coord_t *c2)
{
#if CMK_CRAYXE
	return (c1->mesh_x == c2->mesh_x && c1->mesh_y == c2->mesh_y && c1->mesh_z == c2->mesh_z);
#else
	return 1;
#endif
}

/*
 * check_topology
 * checks if the passed hw_topology struct's members have valid values.
 */
int check_topology(const hw_topology *hw_top)
{
	if(hw_top->core_per_numa < 1)
		return 0;
	if(hw_top->numa_per_cpu  < 1)
		return 0;
	if(hw_top->cpu_per_node < 1)
		return 0;
	if((hw_top->core_per_cpu < 1)
	|| (hw_top->core_per_cpu < hw_top->core_per_numa))
		return 0;
	if((hw_top->core_per_node < 1)
	|| (hw_top->core_per_node < hw_top->core_per_cpu)
	|| (hw_top->core_per_node < hw_top->core_per_numa))
		return 0;
	if(hw_top->node_per_nic < 1)
		return 0;
	if(((hw_top->reserved_cores % 2) != 0)
	|| (hw_top->reserved_cores < hw_top->core_per_node)
	|| (hw_top->reserved_cores < hw_top->core_per_cpu)
	|| (hw_top->reserved_cores < hw_top->core_per_numa))
		return 0;

	//Check that latencies increase in order and are all positive integers.
	/*
	comm_lat lat = hw_top->latencies;
	if((lat.intra_numa < 1)
	|| (lat.intra_numa > lat.intra_cpu)
	|| (lat.intra_cpu > lat.intra_node)
	|| (lat.intra_nic > (lat.inter_node_fix + lat.inter_node_hop)))
		return 0;
*/
	return 1;
}

