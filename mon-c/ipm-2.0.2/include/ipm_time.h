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

#ifndef IPM_TIME_H_INCLUDED
#define IPM_TIME_H_INCLUDED

#include <sys/time.h>
#include <time.h>

extern double ipm_seconds_per_tick;



#define IPM_TIMEVAL( tv_ ) \
  ((tv_.tv_sec)+(tv_.tv_usec)*1.0e-6)

#ifdef HAVE_RDTSC 

#define IPM_TIMESTAMP( time_ )						\
  {									\
    unsigned int low, high;						\
    unsigned long long ticks;						\
    __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));		\
    ticks = high;							\
    ticks = ticks<<32;							\
    ticks += low;							\
    time_ = (double)(ticks) * ipm_seconds_per_tick;			\
  }

#else


/*
  use the macro IPM_TIMESTAMP with a double parameter argument. the
  returned value is time in seconds passed since some point of time in
  the past
*/
#define IPM_TIMESTAMP( time_ )				\
  {							\
    static struct timeval tv;				\
    gettimeofday( &tv, NULL );				\
    time_=IPM_TIMEVAL(tv);				\
  }

#endif /* HAVE_RDTSC */


double ipm_timestamp();

/* wallclock time, based on gettimeofday() */
double ipm_wtime();

double ipm_utime();
double ipm_stime();

double ipm_mtime();
double ipm_iotime(); 
double ipm_mpiiotime();
double ipm_omptime();
double ipm_ompidletime();

double ipm_cudatime();
double ipm_cublastime();
double ipm_cuffttime();

void ipm_time_init(int flags);


#endif /* IPM_TIME_H_INCLUDED */
