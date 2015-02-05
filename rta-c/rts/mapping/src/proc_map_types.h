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

#ifndef PROC_MAP_TYPES_H
#define PROC_MAP_TYPES_H

#if CMK_CRAYXE
#include <rca_types.h>
#define mesh_coord_t rca_mesh_coord_t
#else
#define mesh_coord_t int
#endif

#include <scotch.h>

#define graph_t SCOTCH_Graph
#define arch_t SCOTCH_Arch

typedef int map_status_t;

typedef struct{
	size_t size;
	SCOTCH_Num *mapping;
} mapping_t;

#define MAP_ARG_ERROR 0
#define MAP_IO_ERROR -1
#define MAP_LIB_ERROR -2
#define MAP_ERROR -3
#define MAP_SUCCESS 1

#endif
