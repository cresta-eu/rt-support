/******************************************************************\
* This library is free software: you can redistribute it and/or   *
* modify it under the terms of the GNU LGPL as published by       *       
* the Free Software Foundation, either version 3 of the License,  *
* or (at your option) any later version.                          *
*                                                                 *     
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   *
* GNU Lesser General Public License for more details.             *
*                                                                 *
* You should have received a copy of the GNU Lesser General       *
*  Public License along with this program.  If not,               *
* see <http://www.gnu.org/licenses/>.                             *
\******************************************************************/

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#define IPM_ADDR_TYPE    unsigned long long int
#define IPM_ADDR_TYPEF  "%p"

/*
#define IPM_COUNT_TYPE      unsigned long long int
#define IPM_COUNT_TYPEF     "llu"
#define IPM_COUNT_MPITYPE   MPI_UNSIGNED_LONG_LONG
*/

#define IPM_COUNT_TYPE        unsigned long int
#define IPM_COUNT_TYPEF       "lu"
#define IPM_COUNT_MPITYPE     MPI_UNSIGNED_LONG
#define IPM_COUNT_MAX         4294967295UL

#define IPM_RANK_TYPE        int 
#define IPM_RANK_TYPEF       "d" 




#endif /* TYPES_H_INCLUDED */
