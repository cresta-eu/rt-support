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

#ifndef IPM_INTROSPECT_H_INCLUDED
#define IPM_INTROSPECT_H_INCLUDED

#include "ipm_sizes.h"

/*
 * PIA == Performance Introspection API 
 *
 */

/* for compile-time checks */
#define IPM_HAVE_PIA  1


#define PIA_MAXLEN_LABEL 64

/* return values for functions */
typedef int pia_ret_t;

#define PIA_OK         0
#define PIA_NOTFOUND  -1


/*
 * pia_regid_t is an integer identifier for regions
 *
 * == 0  represents the whole application
 *  < 0  invalid, does not exist, error condition
 *  > 0  a valid user-defined region in IPM
 */
typedef int pia_regid_t;


typedef struct 
{
  pia_regid_t  id;
  char         name[PIA_MAXLEN_LABEL];
  unsigned     count; /* executed how many times? */
  double       wtime; /* wallclock time */
  double       cputime;  /* CPU time */
  double       mtime; /* time in mpi */

#ifdef HAVE_PAPI
  long long    ctr[MAXNUM_PAPI_EVENTS];
  int	       nc; 
#endif

} pia_regdata_t;


#ifdef HAVE_REGHISTO
typedef struct 
{
  double history[MAXSIZE_HISTORY];
  int last_call_index;
} pia_reghistory_t;
#endif


/* 
 * navigate the region hierarchy:
 * 
 * - pia_current_region() returns the id of the region at 
 *   the point of invocation 
 * - pia_child_region() returns the id of the *first* 
 *   sub (child) region
 * - pia_parent_region() returns the id of the 
 *   parent of the current region
 * - pia_next_region() returns the next region on the 
 *   same level of the hierarchy
 * 
 * negative return values indicate that the requested region does not
 * exist
 */
int pia_init(void);
pia_regid_t pia_current_region(void); 
pia_regid_t pia_child_region(pia_regid_t reg);
pia_regid_t pia_parent_region(pia_regid_t reg);
pia_regid_t pia_next_region(pia_regid_t reg);

pia_regid_t pia_find_region_by_name(char *name);

pia_ret_t pia_get_region_data(pia_regdata_t *rdata, pia_regid_t reg);


//double pia_get_region_last_call(pia_regid_t reg);
//pia_ret_t pia_get_region_history(pia_reghistory_t *rdata, pia_regid_t rid);

/*
 * pia_act_t is an integer identifier for IPM activities 
 * an activity is like an MPI or Posix-IO call or time spent
 * inside an OpenMP region
 *
 * => 0  represents a valid activity
 * < 0 represents error, not available, ...
 */
typedef int pia_act_t;

/* #define PIA_ACT_ALL_MPI     0xFFFF */

pia_act_t pia_find_activity_by_name(char *name);


typedef struct 
{
  int ncalls;
  long long bytes;
  double tmin, tmax, tsum;
} pia_actdata_t; 

pia_ret_t pia_init_activity_data(pia_actdata_t *adata);
pia_ret_t pia_get_activity_data(pia_actdata_t *adata, pia_act_t act);
pia_ret_t pia_get_activity_data_by_region(pia_actdata_t *adata, pia_act_t act, int rid);

#endif /* IPM_INTROSPECT_H_INCLUDED */
