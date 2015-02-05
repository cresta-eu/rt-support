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
#include "proc_map_types.h"

void shuffle(int *array, size_t n) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int usec = tv.tv_usec;
	srand48(usec);


	if (n > 1) {
		size_t i;
		for (i = n - 1; i > 0; i--) {
			size_t j = (unsigned int) (drand48()*(i+1));
			int t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

int main(int argc, char **argv)
{
	int i, pid, mapping_size;
	mapping_t *mapping;
	FILE *fp;
	char *mapping_filename;

	MPI_Init(0,0);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_size(MPI_COMM_WORLD, &mapping_size);
	if(argc != 2)
	{
		if(pid == 0)
		{
			printf("usage: %s <mapping file>\n", argv[0]);
		}
		MPI_Finalize();
		return 0;
	} else
	{
		mapping_filename = argv[1];
	}

	if(pid == 0)
	{
		mapping = allocate_mapping(mapping_size);
		printf("shuffling of size: %d\n", mapping->size);
		for(i = 0; i < mapping->size; i++)
		{
			mapping->mapping[i] = i;
		}
		shuffle(mapping->mapping, mapping->size);
		fp = fopen(mapping_filename, "w");
		for(i = 0; i < mapping->size; i++)
		{
			fprintf(fp, "%d %d\n", i, mapping->mapping[i]);
		}
		fclose(fp);
		free_mapping(mapping);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}

