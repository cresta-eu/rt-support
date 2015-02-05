/*
 * mampicl - message passing communication routines library.
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

#ifndef _CR_MPI_INCLUDED_
#define _CR_MPI_INCLUDED_

/* ------------------------------------------------------------------------- */

#if defined(MPI3_ALLOWED)
#define MPI_CPARM const
#else
#define MPI_CPARM
#endif /* MPI3_ALLOWED */

int Cr_Init( MPI_Comm comm );

/**
 * @param sendbuf (I)    starting address of send buffer (choice)
 *
 * @param sendcounts (I) non-negative integer array (of length group size)
 *                       specifying the number of elements to send to each rank
 *
 * @param sdispls (I)    integer array (of length group size). Entry j
 *                       specifies the displacement (relative to sendbuf) from
 *                       which to take the outgoing data destined for process j
 *
 * @param sendtype (I)   data type of send buffer elements (handle)
 *
 * @param recvbuf (O)    address of receive buffer (choice)
 *
 * @param recvcounts (I) non-negative integer array (of length group size)
 *                       specifying the number of elements that can be
 *                       received from each rank
 *
 * @param rdispls (I)    integer array (of length group size). Entry i
 *                       specifies the displacement (relative to recvbuf)
 *                       at which to place the incoming data from process i
 *
 * @param recvtype (I)   data type of receive buffer elements (handle)
 *
 * @param comm (I)       communicator (handle)
 */
int
Cr_Alltoallv(
    MPI_CPARM void* sendbuf, MPI_CPARM int* sendcounts, MPI_CPARM int* sdispls,
        MPI_Datatype sendtype,
    void* recvbuf, MPI_CPARM int* recvcounts, MPI_CPARM int* rdispls,
        MPI_Datatype recvtype,
    MPI_Comm comm);


/**
 * @param sendbuf (I)    starting address of send buffer (choice)
 *
 * @param sendcounts (I) non-negative integer array (of length group size)
 *                       specifying the number of elements to send to each rank
 *
 * @param sdispls (I)    integer array (of length group size). Entry j
 *                       specifies the displacement (relative to sendbuf) from
 *                       which to take the outgoing data destined for process j
 *
 * @param sendtype (I)   data type of send buffer elements (handle)
 *
 * @param recvbuf (O)    address of receive buffer (choice)
 *
 * @param recvcounts (I) non-negative integer array (of length group size)
 *                       specifying the number of elements that can be
 *                       received from each rank
 *
 * @param rdispls (I)    integer array (of length group size). Entry i
 *                       specifies the displacement (relative to recvbuf) at
 *                       which to place the incoming data from process i
 *
 * @param recvtype (I)   data type of receive buffer elements (handle)
 *
 * @param comm (I)       communicator (handle)
 */
int
Cr_Alltoallv2(
    MPI_CPARM void *sendbuf, MPI_CPARM int sendcounts[], MPI_CPARM int sdispls[],
        MPI_Datatype sendtype,
    void *recvbuf, MPI_CPARM int recvcounts[], MPI_CPARM int rdispls[],
        MPI_Datatype recvtype,
    MPI_Comm comm);


/**
 * @param sendbuf (I)      starting address of send buffer (choice)
 *
 * @param sendcounts (I)   non-negative integer array (of length outdegree)
 *                         specifying the number of elements to send to each
 *                         neighbor
 *
 * @param sdispls (I)      integer array (of length outdegree). Entry j
 *                         specifies the displacement (relative to sendbuf)
 *                         from which to send the outgoing data to neighbor j
 *
 * @param sendtype (I)     data type of send buffer elements.
 *
 * @param recvbuf (O)      starting address of receive buffer (choice)
 *
 * @param recvcounts (I)   non-negative integer array (of length indegree)
 *                         specifying the number of elements that are received
 *                         from each neighbor integer array
 *                         (of length indegree).
 *
 * @param rdispls (I)      the displacement (relative to recvbuf) at which to
 *                         the incoming data from neighbor i
 *
 * @param recvtype (I)     Entry i specifies data type of receive
 *                         buffer elements (handle)
 *
 * @param comm (I)         communicator with topology structure (handle)
 */
int
Cr_Neighbor_alltoallv(
    MPI_CPARM void *sendbuf, MPI_CPARM int sendcounts[], MPI_CPARM int sdispls[],
        MPI_Datatype sendtype,
    void *recvbuf, MPI_CPARM int recvcounts[], MPI_CPARM int rdispls[],
        MPI_Datatype recvtype, MPI_Comm comm);

int Cr_Bcast(
    void *buff, int count, MPI_Datatype dtype, int root, MPI_Comm comm );

/* ------------------------------------------------------------------------- */

#endif /* _CR_MPI_INCLUDED_ */
