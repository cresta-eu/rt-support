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
#include <mpi.h>

#include "bits.h"
#include "logging.h"

#include "commprim_aux.h"

/* ------------------------------------------------------------------------- */

int
cread( void *buffer, int nbytes,
       unsigned int inchan, unsigned int outchan, CommContext comm )
{
    L4C(__log4c_category_trace(GlobLogCat,
                           "cread(%x, %d, %x, %x, comm) @ %d(%d) -- begin",
                           buffer, nbytes, inchan, outchan, comm->rank, comm->np));

    int ret = 0;

#ifdef USE_LOG4C
    int _inchan = inchan;
#endif

    for (int bit = 0; inchan != 0; bit += 1, inchan >>= 1)
    {
        if ( TST_BIT(inchan, 0) )
        {
#ifdef USE_LOG4C
            if ( inchan != 1 )
                log4c_category_error(GlobLogCat,
                                 "cread(): wrong inchan mask: %x", _inchan);
#endif
            assert(inchan == 1); // only one bit set

            int src = chan_to_proc(comm->rank, bit);

            L4C(__log4c_category_trace(GlobLogCat,
                            "cread(): channel mask %x => receiving from %d",
                            BIT(bit), src));

            MPI_Status stat;

            MPI_Recv(buffer, nbytes, MPI_BYTE, src, MPI_ANY_TAG,
                     comm->comm, &stat);
            if ( nbytes > 0 )
              MPI_Get_count(&stat, MPI_BYTE, &ret);

            if ( outchan != 0 )
                cwrite(buffer, nbytes, outchan, comm);
            break;
        }
    }
    L4C(__log4c_category_trace(GlobLogCat,
                           "cread() @ %d(%d) ==> %d -- end",
                           comm->rank, comm->np, nbytes));
    return ret;
}


int
ciread( void *buffer, int nbytes,
        unsigned int inchan, unsigned int outchan, CommContext comm )
{
    L4C(__log4c_category_trace(GlobLogCat,
                           "ciread(%x, %d, %x, %x, comm) @ %d(%d) -- begin",
                           buffer, nbytes, inchan, outchan, comm->rank, comm->np));

#ifdef USE_LOG4C
    int _inchan = inchan;
#endif

    for (int bit = 0; inchan != 0; bit += 1, inchan >>= 1)
    {
        if ( TST_BIT(inchan, 0) )
        {
#ifdef USE_LOG4C
            if ( inchan != 1 )
                log4c_category_error(GlobLogCat,
                                 "ciread(): wrong inchan mask: %x", _inchan);
#endif
            assert(inchan == 1); // only one bit set

            int src = chan_to_proc(comm->rank, bit);

            L4C(__log4c_category_trace(GlobLogCat,
                            "ciread(): channel mask %x => receiving from %d",
                            BIT(bit), src));

            MPI_Irecv(buffer, nbytes, MPI_BYTE, src, 0, comm->comm,
                      &comm->reqs[comm->open_reqs++]);

            if ( outchan != 0 )
                ciwrite(buffer, nbytes, outchan, comm);
            break;
        }
    }
    L4C(__log4c_category_trace(GlobLogCat,
                           "ciread() @ %d(%d) ==> %d -- end",
                           comm->rank, comm->np, nbytes));
    return nbytes;
}
