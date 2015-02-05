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

#ifndef MOD_CLUSTERING_H_INCLUDED
#define MOD_CLUSTERING_H_INCLUDED

#include "ipm_modules.h"

typedef struct 
{
  int   myrank;    /* own rank */
  int   clrank;    /* rank of the cluster representative */
  
  /* 
   *  'structural' or algorithmic metrics
   */
  int ncoll;       /* number of collective ops */
  int nroot;       /* number of times as root of the op */
  int np2p;        /* number of p2p ops */
  int npar;        /* number of comm. partners in p2p ops */
  double parloc;   /* 'location' of comm. partners */ 
  double pardist;  /* 'distance' of comm. partners */

  /* 
   * quantitative metrics
   */
  double wallt;     /* wallclock time */
  double gflops;    /* gigaflop rate */
  double tcoll;     /* time in collectives */
  double tp2p;      /* time in p2p */
  double dcoll;     /* data transferred in collectives */
  double dp2p;      /* data transferred in p2p */

} procstats_t;


extern procstats_t mystats;

int mod_clustering_init(ipm_mod_t* mod, int flags);
int mod_clustering_output(ipm_mod_t* mod, int flags);

void print_procstat(int hdr, procstats_t *stats);

#endif /* MOD_CLUSTERING_H_INCLUDED */
