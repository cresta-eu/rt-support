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

#ifndef IPM_DEBUG_H_INCLUDED
#define IPM_DEBUG_H_INCLUDED

#include "perfdata.h"

/*
 * IPMDBG macro
 */


#ifdef HAVE_MPI
/* print with rank ID */
#define PRINTF_DBG(format_,args_...) {					\
    fprintf(stderr, "IPM%3d: "format_, task.taskid, ## args_ ); }
#else
/* print with pid */
#define PRINTF_DBG(format_,args_...) {				\
    fprintf(stderr, "IPM%6ld: "format_, ((long)task.pid), ## args_ ); }
#endif /* HAVE_MPI */


#ifdef DEBUG

#define IPMDBG(format_,args_...) PRINTF_DBG(format_, ## args_)
#define IPMDBG0(format_,args_...) { if(task.taskid==0) { PRINTF_DBG(format_, ## args_); } }

#else 

#define IPMDBG(format_,args_...) 				\
  if( (task.flags)&FLAG_DEBUG ) { PRINTF_DBG(format_, ## args_); } 

#define IPMDBG0(format_,args_...) 				\
  if( (task.flags)&FLAG_DEBUG && (task.taskid==0) ) { PRINTF_DBG(format_, ## args_); } 


#endif /* DEBUG */



/* 
 * IPMERR macro 
 */

#ifdef HAVE_MPI 
#define IPMERR(format_,args_...) { \
    fprintf(stderr, "IPM%3d: ERROR "format_, task.taskid, ## args_ ); }
#else
#define IPMERR(format_,args_...) { \
    fprintf(stderr, "IPM%6ld: ERROR "format_, ((long)task.pid), ## args_ ); }
#endif /* HAVE_MPI */


#ifdef HAVE_MPI 
#define IPMMSG(format_,args_...) { \
    fprintf(stderr, "IPM%3d: "format_, task.taskid, ## args_ ); }
#else
#define IPMMSG(format_,args_...) { \
    fprintf(stderr, "IPM%6ld: "format_, ((long)task.pid), ## args_ ); }
#endif /* HAVE_MPI */


#endif /* IPM_DEBUG_H_INCLUDED */
