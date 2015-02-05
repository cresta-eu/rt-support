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

#include "commprim.h"

#include <assert.h>
#include <memory.h>
#include <mpi.h>

#include "bits.h"
#include "logging.h"

#include "commprim_aux.h"

/* ------------------------------------------------------------------------- */

int
cshift( void *inbuf, int inbytes, unsigned int inchan,
        void *outbuf, int outbytes, unsigned int outchan, CommContext comm )
{
    L4C(__log4c_category_trace(GlobLogCat,
                           "cshift(%x, %d, %x, %x, %d, %x, comm) @ %d(%d) -- begin",
                           inbuf, inbytes, inchan,
                           outbuf, outbytes, outchan, comm->rank, comm->np));
    int ret = -1;

    if ( comm->iseven )
    {
        cwrite(outbuf, outbytes, outchan, comm);
        ret = cread(inbuf, inbytes, inchan, 0, comm);
    }
    else
    {
        // char tmp_buf[1024*1024];

        // ret = cread(tmp_buf, inbytes, inchan, 0, comm);
        ret = cread(inbuf, inbytes, inchan, 0, comm);
        cwrite(outbuf, outbytes, outchan, comm);
        // memcpy(inbuf, tmp_buf, ret);
    }

    L4C(__log4c_category_trace(GlobLogCat,
                           "cshift() @ %d(%d) ==> %d -- end",
                           comm->rank, comm->np, ret));
    return ret;
}

int
cishift( void *inbuf, int inbytes, unsigned int inchan,
         void *outbuf, int outbytes, unsigned int outchan, CommContext comm )
{
    L4C(__log4c_category_trace(GlobLogCat,
                           "cishift(%x, %d, %x, %x, %d, %x, comm) @ %d(%d) -- begin",
                           inbuf, inbytes, inchan,
                           outbuf, outbytes, outchan, comm->rank, comm->np));
    int ret = -1;

//    if ( comm->iseven )
//    {
//        ciwrite(outbuf, outbytes, outchan, comm);
//        ret = ciread(inbuf, inbytes, inchan, 0, comm);
//    }
//    else
    {
        // char tmp_buf[1024*1024];

        // ret = cread(tmp_buf, inbytes, inchan, 0, comm);
        ret = ciread(inbuf, inbytes, inchan, 0, comm);
        ciwrite(outbuf, outbytes, outchan, comm);
        // memcpy(inbuf, tmp_buf, ret);
    }

    MPI_Status stats[comm->open_reqs];

    MPI_Waitall(comm->open_reqs, comm->reqs, stats);
    comm->open_reqs = 0;
    if ( inbytes > 0 )
      // MPI_Get_count(&stats[comm->iseven ? 1 : 0], MPI_BYTE, &ret);
      MPI_Get_count(&stats[0], MPI_BYTE, &ret);

    L4C(__log4c_category_trace(GlobLogCat,
                           "cishift() @ %d(%d) ==> %d -- end",
                           comm->rank, comm->np, ret));
    return ret;
}
