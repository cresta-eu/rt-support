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

#include <string.h>

#include "ipm.h"
#include "ipm_introspect.h"
#include "calltable.h"
#include "hashtable.h"

#ifdef HAVE_PAPI
#include "mod_papi.h"
#endif  /* HAVE_PAPI */ 

struct region_map_elem {
   struct region *reg;
   int id;
   struct region_map_elem *next;
};

struct region_map_elem *regions_map[10];

pia_regid_t pia_init()
{
   int i;

   for(i = 0; i < 10; i++)
	regions_map[i] = NULL;

   return PIA_OK;
}

pia_regid_t pia_current_region()
{
  struct region* reg; 

  reg = ipm_rstackptr;
  if(reg) { 
    return reg->id;
  } else {
    return 0;
  }
}


pia_regid_t pia_child_region(pia_regid_t rid)
{
  struct region* reg; 

  reg = rstack_find_region_by_id(ipm_rstack, rid);
  
  if(reg && reg->child) {
    return reg->child->id;
  } else {
    return -1;
  }
}


pia_regid_t pia_parent_region(pia_regid_t rid)
{
  struct region* reg; 

  reg = rstack_find_region_by_id(ipm_rstack, rid);
  
  if(reg && reg->parent) {
    return reg->parent->id;
  } else {
    return -1;
  }
}


pia_regid_t pia_next_region(pia_regid_t rid)
{
  struct region* reg; 

  reg = rstack_find_region_by_id(ipm_rstack, rid);

  if(reg && reg->next) {
    return reg->next->id;
  } else {
    return -1;
  }
}

pia_regid_t pia_find_region_by_name(char *name)
{
  struct region* reg; 
  struct region_map_elem *map_elem, *map_iter;

  reg = rstack_find_region_by_name(ipm_rstack, name);

  /* Update the map */
  /* TODO Look if already in the map */
  map_elem = (struct region_map_elem *)malloc(sizeof(struct region_map_elem));
  map_elem->reg = reg;
  map_elem->id = reg->id;
  map_elem->next = NULL;

  map_iter = regions_map[(reg->id) & 1];
  if(map_iter == NULL) regions_map[(reg->id) & 1] = map_elem;
  else {
	while ( map_iter->next != NULL )
		map_iter = map_iter->next;

	map_iter->next = map_elem;
  }

  if(reg) { 
    return reg->id;
  } else {
    return -1;
  }
}

void pia_find_region_by_name_(char *name, int *id)
{
  struct region* reg;
  struct region_map_elem *map_elem, *map_iter;

  reg = rstack_find_region_by_name(ipm_rstack, name);

  /* Update the map */
  /* TODO Look if already in the map */
  map_elem = (struct region_map_elem *)malloc(sizeof(struct region_map_elem));
  map_elem->reg = reg;
  map_elem->id = reg->id;
  map_elem->next = NULL;

  map_iter = regions_map[(reg->id) & 1];
  if(map_iter == NULL) regions_map[(reg->id) & 1] = map_elem;
  else {
        while ( map_iter->next != NULL )
                map_iter = map_iter->next;

        map_iter->next = map_elem;
  }

  if(reg) {
    *id = reg->id;
    return;
  } else {
    *id = -1;
    return;
  }
}



pia_ret_t pia_get_region_data(pia_regdata_t *rdata, pia_regid_t rid)
{
  struct region* reg=0; 
  int i;
  struct region_map_elem *map_elem;
 
//  reg = rstack_find_region_by_id(ipm_rstack, rid);  

  map_elem = regions_map[rid & 1];
  while ( map_elem != NULL) {
	if(map_elem->id == rid) {
	   reg = map_elem->reg;
	   break;
	}
	map_elem = map_elem->next;
  }
	    
  if( !reg ) {
    return PIA_NOTFOUND;
  }

  rdata->id=rid;
  strncpy(rdata->name, reg->name, PIA_MAXLEN_LABEL);
  rdata->count=reg->nexecs;
  rdata->wtime=reg->wtime;

// UTIME is NO CPU time !!!!!!!!!!!!!!

  rdata->cputime=reg->utime;
  rdata->mtime=reg->mtime;

#ifdef HAVE_PAPI

  rdata->nc = 0;
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    if( !(papi_events[i].name[0]) )
      continue;
   
    rdata->ctr[i] = reg->ctr[i];
    (rdata->nc)++;
  }

#endif /* HAVE_PAPI */

  return PIA_OK;
}


pia_ret_t pia_get_region_data_by_name(pia_regdata_t *rdata, char *name)
{
  struct region* reg=0;
  int i;

  reg = rstack_find_region_by_name(ipm_rstack, name);
  if( !reg ) {
    return PIA_NOTFOUND;
  }

  rdata->id=reg->id;
  strncpy(rdata->name, reg->name, PIA_MAXLEN_LABEL);
  rdata->count=reg->nexecs;
  rdata->wtime=reg->wtime;

// UTIME is NO CPU time !!!!!!!!!!!!!!

  rdata->cputime=reg->utime;
  rdata->mtime=reg->mtime;

#ifdef HAVE_PAPI

  rdata->nc = 0;
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    if( !(papi_events[i].name[0]) )
      continue;

    rdata->ctr[i] = reg->ctr[i];
    (rdata->nc)++;
  }

#endif /* HAVE_PAPI */

  return PIA_OK;
}


void pia_get_region_data_(int *id, char *name, int *count, double *wtime, double *mtime, long long *counters, int *nc, int *ierr)
{
  struct region* reg=0;
  int i;
  struct region_map_elem *map_elem;


  map_elem = regions_map[*id & 1];
  while ( map_elem != NULL) {
        if(map_elem->id == *id) {
           reg = map_elem->reg;
           break;
        }
        map_elem = map_elem->next;
  }

  if( !reg ) {
    *ierr = PIA_NOTFOUND;
    return;
  }

  strncpy(name, reg->name, PIA_MAXLEN_LABEL);
  *count=reg->nexecs;
  *wtime=reg->wtime;
  *mtime=reg->mtime;

#ifdef HAVE_PAPI

  *nc = 0;
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    if( !(papi_events[i].name[0]) )
      continue;

    counters[i] = reg->ctr[i];
    (*nc)++;
  }

#endif /* HAVE_PAPI */

  *ierr = PIA_OK;
  return;
}


/*
#ifdef HAVE_REGHISTO
pia_ret_t pia_get_region_history(pia_reghistory_t *rdata, pia_regid_t rid)
{ 

  struct region* reg=0;
  int i;

  reg = rstack_find_region_by_id(ipm_rstack, rid);
  if( !reg ) {
    return PIA_NOTFOUND;
  }

  for( i=0; i<=reg->last_call_index; i++ ) 
     rdata->history[i] = reg->history[i];
  
  rdata->last_call_index = reg->last_call_index;

  return PIA_OK;
}

double pia_get_region_last_call(pia_regid_t rid)
{
  struct region* reg=0;

  reg = rstack_find_region_by_id(ipm_rstack, rid);
  if( !reg ) {
    return PIA_NOTFOUND;
  }

  if( reg->last_call_index < 0 ) return 0;

  return( reg->history[reg->last_call_index] );
}

#endif
*/

pia_act_t pia_find_activity_by_name(char *name)
{
  int i;

  for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    if( ipm_calltable[i].name != NULL && !strcmp(name, ipm_calltable[i].name) ) {
      return i; /* i will be >= 0 */
    }
  }
  
  return PIA_NOTFOUND;
}


void pia_find_activity_by_name_(int *id, char *name)
{
  int i;


  for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    if( !strcmp(name, ipm_calltable[i].name) ) {
      *id = i; /* i will be >= 0 */
      return;
    }
  }
  *id = PIA_NOTFOUND;
  return;
}


pia_ret_t pia_init_activity_data(pia_actdata_t *adata)
{
  adata->ncalls=0;
  adata->bytes=0;
  adata->tmin=1.0e15;
  adata->tmax=0.0;
  adata->tsum=0.0;
  
  return PIA_OK;
}

void pia_init_activity_data_(int *ncalls, double *tmin, double *tmax,
					double *tsum, int *error)
{
  *ncalls=0;
  *tmin=1.0e15;
  *tmax=0.0;
  *tsum=0.0;
 
  *error = PIA_OK;
}


pia_ret_t pia_get_activity_data(pia_actdata_t *adata,
				pia_act_t act)
{
  int i;
  int bytes;
  int rank;
  
  for( i=0; i<MAXSIZE_HASH; i++ ) {
    if( ipm_htable[i].count==0 ) 
      continue;
    
    if( KEY_GET_ACTIVITY(ipm_htable[i].key)==act ) {
	  
      bytes = KEY_GET_BYTES(ipm_htable[i].key);
      rank  = KEY_GET_RANK(ipm_htable[i].key);
    
      if( ipm_htable[i].t_min < adata->tmin ) 
	adata->tmin = ipm_htable[i].t_min;
      if( ipm_htable[i].t_max > adata->tmax ) 
	adata->tmax = ipm_htable[i].t_max;
      
      adata->tsum+=ipm_htable[i].t_tot;
      adata->ncalls+=ipm_htable[i].count;
      adata->bytes+=bytes;
    }
  }

  return PIA_OK;
}


pia_ret_t pia_get_activity_data_by_region(pia_actdata_t *adata,
                                pia_act_t act, int rid)
{
  int i;
  int bytes;
  int rank;

  for( i=0; i<MAXSIZE_HASH; i++ ) {
    if( ipm_htable[i].count==0 )
      continue;

    if( (KEY_GET_ACTIVITY(ipm_htable[i].key)==act) && (KEY_GET_REGION(ipm_htable[i].key) == rid)) {

      bytes = KEY_GET_BYTES(ipm_htable[i].key);
      rank  = KEY_GET_RANK(ipm_htable[i].key);

      if( ipm_htable[i].t_min < adata->tmin )
        adata->tmin = ipm_htable[i].t_min;
      if( ipm_htable[i].t_max > adata->tmax )
        adata->tmax = ipm_htable[i].t_max;

      adata->tsum+=ipm_htable[i].t_tot;
      adata->ncalls+=ipm_htable[i].count;
    }
  }

  return PIA_OK;
}


void pia_get_activity_data_(int *ncalls, double *tmin, double *tmax,
                                double *tsum, int *act, int *error)
{
  int i;
  int bytes;
  int rank;

  for( i=0; i<MAXSIZE_HASH; i++ ) {
    if( ipm_htable[i].count==0 )
      continue;

    if( KEY_GET_ACTIVITY(ipm_htable[i].key)== *act ) {

      bytes = KEY_GET_BYTES(ipm_htable[i].key);
      rank  = KEY_GET_RANK(ipm_htable[i].key);

      if( ipm_htable[i].t_min < *tmin )
        *tmin = ipm_htable[i].t_min;
      if( ipm_htable[i].t_max > *tmax )
        *tmax = ipm_htable[i].t_max;

      *tsum+=ipm_htable[i].t_tot;
      *ncalls+=ipm_htable[i].count;
    }
  }
  *error = PIA_OK;
}

