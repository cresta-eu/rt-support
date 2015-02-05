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

#ifndef SIZES_H_INCLUDED
#define SIZES_H_INCLUDED

/* other prime number hash table sizes:
   
   #define MAXSIZE_HASH              4049
   #define MAXSIZE_HASH              8093
   #define MAXSIZE_HASH             16301
   #define MAXSIZE_HASH             32573
   #define MAXSIZE_HASH             65437
   
*/

#define MAXSIZE_HASH             65437

#ifdef HAVE_KEYHIST
#define MAXSIZE_XHASH            32573
#endif 


#define MAXSIZE_HOSTNAME         16
#define MAXSIZE_USERNAME         16
#define MAXSIZE_ALLOCATIONNAME   16
#define MAXSIZE_JOBID            32
#define MAXSIZE_MACHNAME         32
#define MAXSIZE_MACHINFO         32
#define MAXSIZE_REGLABEL         32
#define MAXSIZE_CMDLINE          4096
#define MAXSIZE_FILENAME         256
#define MAXNUM_REGIONS           256
#define MAXNUM_REGNESTING         32


#define MAXNUM_MODULES           16

/* module MPI */
#define MAXNUM_MPI_OPS           16
#define MAXNUM_MPI_TYPES         64

/* module callpath */
#define MAXSIZE_CALLSTACKDEPTH   30 
#define MAXSIZE_CALLTABLE        1024
#define MAXSIZE_CALLLABEL        64
#define MAXNUM_CALLSITES         8192

/* module keyhist */
#define MAXSIZE_CYCLE            128
#define MAXNUM_CYCLES            128

/* module papi */
#define MAXNUM_PAPI_EVENTS      16
#define MAXNUM_PAPI_COUNTERS    8
#define MAXNUM_PAPI_COMPONENTS  8 
#define MAXSIZE_PAPI_EVTNAME    32

/* module omptracepoints */
#define MAXNUM_THREADS          128


#endif /* IPM_SIZES_INCLUDED */
