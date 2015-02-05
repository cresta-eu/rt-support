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
#include <scotch.h>

#define LINE_LEN 400

int allocate_graph_structs(SCOTCH_Num vertnbr, SCOTCH_Num edgenbr, SCOTCH_Num **vendtab, SCOTCH_Num **vlbltab, SCOTCH_Num **velotab, SCOTCH_Num **verttab, SCOTCH_Num **edgetab, SCOTCH_Num **edlotab)
{
	if((*velotab = malloc(sizeof(SCOTCH_Num) * vertnbr)) != NULL)
	{
		if((*vlbltab = malloc(sizeof(SCOTCH_Num) * vertnbr)) != NULL)
		{
			if((*verttab = malloc(sizeof(SCOTCH_Num) * (vertnbr + 1))) != NULL)
			{
				if((*edgetab = malloc(sizeof(SCOTCH_Num) * edgenbr)) != NULL)
				{
					if((*edlotab = malloc(sizeof(SCOTCH_Num) * edgenbr)) != NULL)
					{
						return 1;
					}
					free(edgetab);
				}
				free(verttab);
			}
			free(vlbltab);
		}
		free(velotab);
	}
	return 0;
}

int main(int argc, char *argv[]){
	FILE* fp = NULL;
	char *line = malloc(LINE_LEN + 1);
	char *save_filename = NULL;
	int i, j, k;
	int current_vertex, vert_from, vert_to, edge_weight;
	size_t size = LINE_LEN;
	int receiving_vertex;
	int receiving_edge_weight;
	int total_weight;
	SCOTCH_Num vertnbr, edgenbr;
	SCOTCH_Num *vendtab = NULL;
	SCOTCH_Num *vlbltab = NULL;
	SCOTCH_Num *velotab = NULL;
	SCOTCH_Num *verttab = NULL;
	SCOTCH_Num *edgetab = NULL;
	SCOTCH_Num *edlotab = NULL;
	SCOTCH_Graph *graph = NULL;
	if(argc != 2)
	{
		printf("usage: %s <output filename>", argv[0]);
		return -1;
	} else
	{
		save_filename = argv[1];
	}

	if(getline(&line, &size, stdin) != -1)
	{
		vertnbr = atoi(line);
	}
	if(getline(&line, &size, stdin) != -1)
	{
		edgenbr = atoi(line);
	}

	if(allocate_graph_structs(vertnbr, edgenbr, &vendtab, &vlbltab, &velotab, &verttab, &edgetab, &edlotab))
	{
		vendtab = (verttab +1);
		for(i=0; i < vertnbr; i++)
		{
			vlbltab[i] = i;
			velotab[i] = 1;
		}
		current_vertex = -1;
		for(i=0; i < edgenbr; i++)
		{
			if(getline(&line, &size, stdin) != -1)
			{
				sscanf(line, "%i %i %i", &vert_from, &vert_to, &edge_weight);
				if(current_vertex != vert_from)
				{
					verttab[vert_from] = i;
					current_vertex++;
				}
				edgetab[i] = vert_to;
				edlotab[i] = edge_weight;
			} else
			{
				return -1;
			}

		}
		fclose(fp);
		for(i=0; i < vertnbr;i++)
		{
			for(j=0; j < verttab[i+1] - verttab[i]; j++)
			{
				receiving_vertex = edgetab[verttab[i]+j];
				receiving_edge_weight = 0;
				for(k = 0; k < 4; k++)
				{
					if(edgetab[verttab[receiving_vertex] + k] == i)
					{
						receiving_edge_weight = edlotab[verttab[receiving_vertex] + k];
						/*
						if( edlotab[verttab[i]+j] != receiving_edge_weight)
						{
							printf("!!! inconsisten edge load between %d %d, %d, %d\n", i, receiving_vertex, edlotab[verttab[i]+j], edlotab[verttab[receiving_vertex] + k]);
						}
						*/
						total_weight = edlotab[verttab[i]+j] + edlotab[verttab[receiving_vertex] + k];
						total_weight /= 2;
						edlotab[verttab[i]+j] = total_weight;
						edlotab[verttab[receiving_vertex] + k] = total_weight;
					}
				}
			}
		}
		verttab[vertnbr] = edgenbr;
		if(NULL != (graph = SCOTCH_graphAlloc())){
		   if(0 == SCOTCH_graphInit(graph))
		   {
			   if(0 == SCOTCH_graphBuild(graph,
						   0,
						   vertnbr,
						   verttab,
						   vendtab,
						   velotab,
						   vlbltab,
						   edgenbr,
						   edgetab,
						   edlotab))
			   {
				   fp = fopen(save_filename, "w");
				   SCOTCH_graphSave(graph, fp);
				   fclose(fp);
			   }
			   SCOTCH_graphExit(graph);
			   free(edgetab);
			   free(verttab);
			   free(edlotab);
			   free(vlbltab);
		   }
		}
	}
	return 0;
}
