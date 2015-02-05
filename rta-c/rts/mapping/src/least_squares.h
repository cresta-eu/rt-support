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

#ifndef POLYNOMIALS_H
#define POLYNOMIALS_H


/*
 * least_square_method_deg_2
 * solves an overdetermined system of a degree 2 polynomial equations on form (A c ≈ y) by using
 * the least square method to find the solution with the lowest residual vector sum.
 * The sizes of the matrices in the system are: A=[n x 2], c=[2 x 1], y=[n x 1], where n is the
 * number of equations. The least square method computes the transpose of A (reffered to here as A')
 * and multiplies both parts with A' from left, forming: A' A c = A' y. This gives a system with exactly
 * one solution, which is here gotten from simple Gaussian elimination.
 *
 * Arguments:	const int num_equations, the number of equations in the system.
 *				const int **equations: pointer to three integer arrays of size num_equations, where the pointed to arrays form the
 *					columns of A (the first two) and y (the third).
				int **c: array of integer pointers, of size two, stores the solution to the system uppon completion
				int *res: the sum of the squared residuals (gives a number to the error of the solution.)
 */
int least_squares(const int num_equations, const int **equations, int **c, int *res);

#endif

