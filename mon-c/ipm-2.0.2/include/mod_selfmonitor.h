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

#ifndef MOD_SELFMONITOR_H_INCLUDED
#define MOD_SELFMONITOR_H_INCLUDED

#include "ipm.h"
#include "ipm_types.h"
#include "ipm_sizes.h"

struct ipm_module;

typedef struct 
{
  unsigned mem_alloc;    /* bytes allocated */
  unsigned num_alloc;    /* number of allcoation calls */
  unsigned num_free;     /* number of free() calls */
  unsigned bytes_send;   /* number of bytes sent using MPI */
  unsigned bytes_recv;   /* number of bytes recv using MPI */
  unsigned num_mpi;      /* number of MPI calls */
  double t_mpi_p2p;      /* time in MPI p2p ops */
  double t_mpi_coll;     /* time in MPI collective calls */
  double t_finalize;     /* total time in MPI_Finalize */
  double t_init;         /* total time in MPI_Init */
} selfmon_t;

extern selfmon_t ipm_selfmon;

/*
int mod_selfmonitor_init(ipm_module* mod, int flags);
int mod_selfmonitor_output(ipm_module* mod, int flags);
*/

/*
 * Prototypes and name mappings for memory 
 * allocation and deallocation routines used by IPM
 * If self-monitoring is not enabled the IPM_xxx names are
 * just replaced by the original functions, incurring 
 * no overhead. If selfmonitoring is on, we call the ipm_xxx 
 * version that keeps track of some information
 */ 
void *ipm_calloc(size_t nmemb, size_t size);
void *ipm_malloc(size_t size);
void ipm_free(void *ptr);
void *ipm_realloc(void *ptr, size_t size);

#ifdef HAVE_SELFMONITOR
#define IPM_CALLOC   ipm_calloc
#define IPM_MALLOC   ipm_malloc
#define IPM_FREE     ipm_free
#define IPM_REALLOC  ipm_realloc
#else
#define IPM_CALLOC   calloc
#define IPM_MALLOC   malloc
#define IPM_FREE     free
#define IPM_REALLOC  realloc
#endif /* HAVE_SELFMONITOR */


/*
 * Prototypes and name mappings for MPI functions 
 * used by IPM itself
 */

#ifdef HAVE_MPI
#include <mpi.h>

int ipm_bcast(void *buffer, int count, MPI_Datatype datatype, int root, 
	      MPI_Comm comm);
int ipm_allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype, 
		  MPI_Comm comm);
int ipm_gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
	       void *recvbuf, int recvcount, MPI_Datatype recvtype, 
	       int root, MPI_Comm comm);
int ipm_reduce(void *sendbuf, void *recvbuf, int count, 
	       MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int ipm_allreduce(void *sendbuf, void *recvbuf, int count, 
		  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int ipm_send(void *buf, int count, MPI_Datatype datatype, int dest, 
	     int tag, MPI_Comm comm);
int ipm_recv(void *buf, int count, MPI_Datatype datatype, int source, 
	     int tag, MPI_Comm comm, MPI_Status *status);

#ifdef HAVE_SELFMONITOR

#define IPM_BCAST       ipm_bcast
#define IPM_ALLGATHER   ipm_allgather
#define IPM_GATHER      ipm_gather
#define IPM_REDUCE      ipm_reduce
#define IPM_ALLREDUCE   ipm_allreduce
#define IPM_SEND        ipm_send
#define IPM_RECV        ipm_recv

#else

#define IPM_BCAST       PMPI_Bcast
#define IPM_ALLGATHER   PMPI_Allgather
#define IPM_GATHER      PMPI_Gather
#define IPM_REDUCE      PMPI_Reduce
#define IPM_ALLREDUCE   PMPI_Allreduce
#define IPM_SEND        PMPI_Send
#define IPM_RECV        PMPI_Recv

#endif /* HAVE_SELFMONITOR */

#else /* HAVE_MPI */

#define IPM_BCAST       
#define IPM_ALLGATHER   
#define IPM_GATHER      
#define IPM_REDUCE      
#define IPM_ALLREDUCE   
#define IPM_SEND        
#define IPM_RECV        




#endif /*HAVE_MPI */




#endif /* MOD_SELFMONITOR_H_INCLUDED */
