/*
 * rta-c - runtime system administration component.
 *  Copyright (C) 2014  Michael Schliephake
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


#ifndef _MPI_WRAPPER_INCLUDED_
#define _MPI_WRAPPER_INCLUDED_

#ifndef MPI_WRAPPER_IMPLEMENTATION
/* RTS client modules */
#if MPIRTS_COMPILERWRAP

#define MPI_Init        __wrap_MPI_Init
#define MPI_Init_thread __wrap_MPI_Init_thread
#define MPI_Finalize    __wrap_MPI_Finalize
#define MPI_Isend       __wrap_MPI_Isend
#define MPI_Recv        __wrap_MPI_Recv
#define MPI_Send        __wrap_MPI_Send

#endif /* MPIRTS_COMPILERWRAP */

#else /* if MPI_WRAPPER_IMPLEMENTATION */
/* Within in the RTS wrapper layer */

#if MPIRTS_LINKWRAP == 1

#if defined(RTS_USE_PMPI)

#define __wrap_MPI_Init MPI_Init
#define __real_MPI_Init PMPI_Init

#define __wrap_MPI_Init_thread MPI_Init_thread
#define __real_MPI_Init_thread PMPI_Init_thread

#define __wrap_MPI_Finalize MPI_Finalize
#define __real_MPI_Finalize PMPI_Finalize

#define __wrap_MPI_Isend MPI_Isend
#define __real_MPI_Isend PMPI_Isend

#define __wrap_MPI_Recv MPI_Recv
#define __real_MPI_Recv PMPI_Recv

#define __wrap_MPI_Send MPI_Send
#define __real_MPI_Send PMPI_Send


#elif defined(__gnu_linux__)


#define MPI_Init         __real_MPI_Init
#define MPI_Init_thread  __real_MPI_Init_thread
#define MPI_Finalize     __real_MPI_Finalize

#define MPI_Isend        __real_MPI_Isend
#define MPI_Recv         __real_MPI_Recv
#define MPI_Send         __real_MPI_Send

#else

#error "Linkwrapping not possible for this toolchain."

#endif /* __gnu_linux__ */

#endif /* MPIRTS_LINKWRAP == 1 */

#if MPIRTS_COMPILERWRAP == 1

#define __real_MPI_Init MPI_Init
#define __real_MPI_Init_thread MPI_Init_thread
#define __real_MPI_Finalize MPI_Finalize
#define __real_MPI_Isend MPI_Isend
#define __real_MPI_Recv MPI_Recv
#define __real_MPI_Send MPI_Send

#endif /* MPIRTS_COMPILERWRAP == 1 */


#endif /* MPI_WRAPPER_IMPLEMENTATION */


#include <mpi.h>

#endif /* _MPI_WRAPPER_INCLUDED_ */
