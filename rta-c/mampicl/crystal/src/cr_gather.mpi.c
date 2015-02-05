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

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "cr_internal.h"
#include "cr_mpi.h"
#include "crystal_router.h"
#include "crystal_aux.h"
#include "logging.h"

/* ------------------------------------------------------------------------- */

#pragma GCC diagnostic ignored "-Wunused-parameter"

/* ------------------------------------------------------------------------- */

typedef struct {
    const void *sendbuf;
    const int  *sendcounts;
    const int  *sdispls;
    // MPI_Datatype sendtype;
    int  sendsize;

    void *recvbuf;
    const int  *recvcounts;
    const int  *rdispls;
    // MPI_Datatype recvtype;
    int recvsize;

    int nnbrs, maxnbrs;
    int *nbr_rank;

    int myrank, numranks;

} Allgatherv_Parms;

static Allgatherv_Parms Agv_parms;

/* ------------------------------------------------------------------------- */

#ifdef USE_LOG4C
static void
log_ag_parms(
    const char *title, int n,
    const int *sendcounts, const int *sdispls,
    const int *recvcounts, const int *rdispls )
{
    __log4c_category_trace(GlobLogCat, "%s", title);
    L4C(__log4c_category_trace(GlobLogCat, "%10s%10s\t%10s%10s",
                               "scounts", "sdispls", "rcounts", "rdispls"));
    for (int i = 0; i < n; i++)
        __log4c_category_trace(GlobLogCat, "%10d%10d\t%10d%10d",
                    sendcounts[i], sdispls[i], recvcounts[i], rdispls[i]);
}
#endif /* USE_LOG4C */

static void
create_msg(
    CrBuffer *buffer, int dest,
    const void *src, int size, int count, int displ, int myrank )
{
    // add message to the out buffer
    CrMsgHeader *msg  = crb_create_msg(buffer);

    msg->ndest  = 0;
    msg->src    = myrank;
    msg->nbytes = count*size;
    msg->status = CURRENT;
    msg->dests[msg->ndest++] = dest;

    crb_add_msgcontent(msg, src + displ*size, msg->nbytes);
    crb_close_msg(buffer, msg);
}


static void
cp_msg_param2buff(
    CrBuffer *buffer, const void *src, int size, const int *counts,
    const int *displs, CrRouter *cr )
{
    for (int i = 0; i < cr->nproc; i++)
        if ( counts[i] > 0 )
            create_msg(buffer, i, src, size, counts[i], displs[i],
                       cr->procnum);
}


static void
cp_msg_buff2param(
    CrBuffer *buffer,
    void *dest, int size, const int *counts, const int *displs, CrRouter *cr)
{
    for (int i = 0; i < cr->nproc; i++)
    {
        if ( counts[i] > 0 )
        {
            int src = i, nrecvd;

            get_message(buffer,
                        cr->procnum, &src, &nrecvd, dest + displs[i]*size);
        }
    }
}


int
Cr_Alltoallv(
    MPI_CPARM void *sendbuf, MPI_CPARM int *sendcounts, MPI_CPARM int *sdispls,
        MPI_Datatype sendtype,
    void *recvbuf, MPI_CPARM int *recvcounts, MPI_CPARM int * rdispls,
        MPI_Datatype recvtype,
    MPI_Comm comm)
{
    if (sendbuf == MPI_IN_PLACE)
    {
        sendcounts = recvcounts;
        sdispls    = rdispls;

        sendbuf = recvbuf;
    }

    // TODO: Flexible type parameter
    int size;

    MPI_Type_size(sendtype, &size);
    crb_clear(&Crr->out_buf);
    cp_msg_param2buff(&Crr->out_buf, sendbuf, size, sendcounts, sdispls, Crr);
    crb_clear(&Crr->in_buf);

    crystal_router(Crr);

    cp_msg_buff2param(&Crr->in_buf, recvbuf, size, recvcounts, rdispls, Crr);
    crb_clear(&Crr->in_buf);

    return 0;
}

static void
read_msg_func( CrBuffer *buff, int dim, void *userdata )
{
    Allgatherv_Parms *agvp = userdata;

    register int mask = 1<<dim;            // requested channel
    register int bit  = agvp->myrank&mask; // != 0 if channel bit set in my procnum

    // L4C(__log4c_category_trace(GlobLogCat,
    //         "  copy_msg_func():\tdim/mask/bit %x/%x/%x", dim, mask, bit));

    // TODO: create array of indices with sendcounts != 0
    //       ==> reduces function complexity from O(# ranks) to
    //           O(# messages to send)
    for (int i = 0; i < agvp->nnbrs; i++)
    {
        // L4C(__log4c_category_trace(GlobLogCat,
        //         "copy_msg_func():\t send %s to %d with %d bytes",
        //         ((dest & mask)^bit)? "   ":"not",
        //         dest, Agv_parms.sendcounts[dest]));
        register int dest = agvp->nbr_rank[i];

        if ( dest >= 0 && ((dest & mask)^bit) && (agvp->sendcounts[dest] > 0) )
        {
            create_msg(buff, dest, agvp->sendbuf, agvp->sendsize,
                       agvp->sendcounts[dest], agvp->sdispls[dest],
                       agvp->myrank);
            agvp->nbr_rank[i] = -1;
        }
    }
}


static int
write_msg_func( CrMsgHeader *msg_hd, int dest, void *userdata )
{
    Allgatherv_Parms *agvp = userdata;

    int size    = agvp->recvsize; // from Agv_parms.recvtype.
    int msg_src = msg_hd->src;
    int msg_len = (msg_hd->nbytes > agvp->recvcounts[msg_src]*size)
                    ? agvp->recvcounts[msg_src]*size : msg_hd->nbytes;

    memcpy((char *)agvp->recvbuf + agvp->rdispls[msg_src]*size,
           cr_msg_content(msg_hd), msg_len);

    return 0;
}


int
Cr_Alltoallv2(
    MPI_CPARM void* sendbuf, MPI_CPARM int* sendcounts, MPI_CPARM int* sdispls,
        MPI_Datatype sendtype,
    void* recvbuf, MPI_CPARM int* recvcounts, MPI_CPARM int* rdispls,
        MPI_Datatype recvtype,
    MPI_Comm comm)
{
    if (sendbuf == MPI_IN_PLACE)
    {
        sendcounts = recvcounts;
        sdispls    = rdispls;

        sendbuf = recvbuf;
    }

#ifdef DEBUG
    // Check for symmetry in the arguments according standard.
#endif

    // TODO: Flexible type parameter
    int   myrank = Crr->comm->rank;
    const void *src = sendbuf;
    void *dest   = recvbuf;
    int   size;

    MPI_Type_size(sendtype, &size);
    Agv_parms.sendbuf    = sendbuf;
    Agv_parms.sendcounts = sendcounts;
    Agv_parms.sdispls    = sdispls;
    // Agv_parms.sendtype   = sendtype;
    Agv_parms.sendsize   = size;

    Agv_parms.recvbuf    = recvbuf;
    Agv_parms.recvcounts = recvcounts;
    Agv_parms.rdispls    = rdispls;
    // Agv_parms.recvtype   = recvtype;
    Agv_parms.recvsize   = size;        // TODO: consider recvtype

    Agv_parms.myrank     = Crr->procnum;
    Agv_parms.numranks   = Crr->nproc;

    if ( Agv_parms.maxnbrs < Agv_parms.numranks )
    {
        free(Agv_parms.nbr_rank);
        Agv_parms.maxnbrs    = Agv_parms.numranks;
        Agv_parms.nbr_rank = malloc(Agv_parms.maxnbrs*sizeof(int));
    }
    Agv_parms.nnbrs = 0;
    for (int i = 0; i < Agv_parms.maxnbrs; i++)
        if ( sendcounts[i] > 0 )
            Agv_parms.nbr_rank[Agv_parms.nnbrs++] = i;

    Crr->read_msg_func   = read_msg_func;
    Crr->write_msg_func  = write_msg_func;
    Crr->userdata        = &Agv_parms;

    // Send to self if necessary
    if ( src != dest && sendcounts[myrank] > 0)
        memcpy((char *)dest + rdispls[myrank]*size,
               (char *)src  + sdispls[myrank]*size,
               sendcounts[myrank]*size);

    crb_clear(&Crr->out_buf);
    crb_clear(&Crr->in_buf);

    crystal_router(Crr);

    crb_clear(&Crr->in_buf);

    Crr->read_msg_func  = NULL;
    Crr->write_msg_func = NULL;

    return 0;
}
