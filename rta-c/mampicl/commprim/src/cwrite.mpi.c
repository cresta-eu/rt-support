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

#include <mpi.h>

#include "bits.h"
#include "logging.h"

#include "commprim_aux.h"

/* ------------------------------------------------------------------------- */

int
cwrite( void *buffer, int nbytes, unsigned int chan, CommContext comm )
{
    L4C(__log4c_category_trace(GlobLogCat,
                           "cwrite(%x, %d, %x, comm) @ %d(%d) -- begin",
                           buffer, nbytes, chan, comm->rank, comm->np));

    for (int bit = 0; chan != 0; bit += 1, chan >>= 1)
    {
        if ( TST_BIT(chan, 0) )
        {
            int dest = chan_to_proc(comm->rank, bit);

            L4C(log4c_category_debug(GlobLogCat,
                                 "cwrite(): bit %x => sending to %d",
                                 BIT(bit), dest));
            MPI_Send(buffer, nbytes, MPI_BYTE, dest, 0, comm->comm);

        }
    }
    L4C(__log4c_category_trace(GlobLogCat,
                           "cwrite() @ %d(%d) ==> %d -- end",
                           comm->rank, comm->np, nbytes));
    return nbytes;
}

int
ciwrite( void *buffer, int nbytes, unsigned int chan, CommContext comm )
{
    L4C(__log4c_category_trace(GlobLogCat,
                           "ciwrite(%x, %d, %x, comm) @ %d(%d) -- begin",
                           buffer, nbytes, chan, comm->rank, comm->np));

    for (int bit = 0; chan != 0; bit += 1, chan >>= 1)
    {
        if ( TST_BIT(chan, 0) )
        {
            int dest = chan_to_proc(comm->rank, bit);

            L4C(log4c_category_debug(GlobLogCat,
                                 "ciwrite(): bit %x => sending to %d",
                                 BIT(bit), dest));
            MPI_Isend(buffer, nbytes, MPI_BYTE, dest, 0, comm->comm,
                      &comm->reqs[comm->open_reqs++]);

        }
    }
    L4C(__log4c_category_trace(GlobLogCat,
                           "ciwrite() @ %d(%d) ==> %d -- end",
                           comm->rank, comm->np, nbytes));
    return nbytes;
}
