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
#include <math.h>

#include "definitions.h"
#include "least_squares.h"

int least_squares(int num_equations, const int **equations, int **c, int *res)
{
	int i,j,k,sum;
	int at_a[2][2]; /* the matrix product of A-transpose multiplied with A*/
	int at_c[2];

	#ifdef DEBUG
	printf("least squaresing\n");
	printf("data:\n A: \n");
	for(i = 0; i < num_equations;i++)
	{
		printf("%d\t%d\n",equations[0][i], equations[1][i]);
	}
	printf("B:\n");
	for(i=0; i < num_equations;i++)
	{
		printf("%d\n",equations[2][i]);
	}
	#endif
	for(i = 0;i < 2; i++) /* every column */
	{
		for(j = 0; j < 2; j++) /* every row */
		{
			sum = 0;
			for(k = 0; k < num_equations; k++)
			{
			 	sum += (equations[i][k] * equations[j][k]);
			}
			at_a[j][i] = sum;
		}
	}
	for(i = 0;i < 2; i++) /* row */
	{
		sum = 0;
		for(k = 0; k < num_equations; k++)
		{
		 	sum += (equations[i][k] * equations[2][k]);
		}
		at_c[i] = sum;
	}
	if(at_a[0][0] < at_a[0][1]) /* let the biggest value on the first column be on the first row */
	{
		i = at_a[0][0];
		at_a[0][0] = at_a[0][1];
		at_a[0][1] = i;
		i = at_a[1][0];
		at_a[1][0] = at_a[1][1];
		at_a[1][1] = i;
		i = at_c[0];
		at_c[0] = at_c[1];
		at_c[1] = i;
	}
	#ifdef DEBUG
	printf("A_T * A:\n");
	printf("%d %d \n%d %d \n", at_a[0][0],at_a[1][0],at_a[0][1],at_a[1][1]);
	printf("A_T * c:\n");
	printf("%d\n%d\n", at_c[0], at_c[1]);
	#endif
	/* Solve the equational system.
	 * Derivation of formulae:
	 * equational system A x = c
	 * The augmented matrix looks like:
	 *
	 * a	b | y1
	 * c	d | y2
	 * or, expressed with the datastructures here:
	 * at_a[0][0]	at_a[1][0] | at_c[0]
	 * at_a[0][1]	at_a[1][1] | at_c[0]
	 *
	 * And Gauss elimination is done starting with dividing the lower row with c, giving:
	 *
	 * a	b 		| y1
	 * 1	d / c	| y2 / c
	 *
	 * From here, a-times the lower row is subtracted from the first row:
	 *
	 * 0    b - a * d / c	| y1 - a * y2 / c
	 * 1	d / c			| y2 / c
	 *
	 * Which gives x2 = (y1 - a * y2 / c) / (b - a * d / c)
	 * And x1 is easily obtained from here, with:
	 *
	 * x1 = (y1 - b*x2) / a
	 * or from:
	 * x1 = (y2 - d*x2) / c
	*/

/*
	printf("x2= %d / %d \n", at_c[0] - (int)((double) (at_a[0][0] * at_c[1]) / at_a[0][1]), at_a[1][0] - (int)((double)(at_a[0][0] * at_a[1][1]) / at_a[0][1]));
	printf("VL: %d - (%d/%d)\n", at_c[0], at_a[0][0] * at_c[1], at_a[0][1]);
	printf("HL: %d - %d\n", at_a[1][0], (int) ((double)(at_a[0][0] * at_a[1][1]) / at_a[0][1]));
*/

	if(at_a[1][0] - ((int) ((double)(at_a[0][0] * at_a[1][1]) / at_a[0][1])) == 0) /* would lead to division by zero */
	{
		fprintf(stderr, "Collected data is not sufficient for least fitting, consider adding more nodes to the test!\n");
		return -1;
	}
	*c[1] = (int) (at_c[0] - (int)((double) (at_a[0][0] * at_c[1]) / at_a[0][1])) / ((double) at_a[1][0] - ((double)(at_a[0][0] * at_a[1][1]) / at_a[0][1]));
	*c[0] = (int) ((double)(at_c[0] - (at_a[1][0] * *c[1])) / at_a[0][0]);
	/* Compute the square residual sum */
	*res = 0;
	for(i = 0; i < num_equations; i++)
	{
		*res += (int) pow(equations[2][i] - ((equations[0][i] * *c[0]) + (equations[1][i] * *c[1])), 2);
	}

	return 0;
}

