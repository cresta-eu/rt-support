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

#include "bits.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

int
commcntxt_size( CommContext comm )
{
    return comm->np;
}


int
commcntxt_id( CommContext comm )
{
    return comm->rank;
}


CommContext
mcMPICommContext( MPI_Comm comm )
{
    CommContext ret = malloc(sizeof(struct _comm_cntxt));

    memset(ret, 0, sizeof(struct _comm_cntxt));
    ret->comm = comm;

    MPI_Comm_rank(comm, &ret->rank);
    MPI_Comm_size(comm, &ret->np);

    int i, procnum;

    /* Count bits in rank number */
    for (i = 0, procnum = ret->rank; procnum != 0; procnum >>= 1)
        if ( TST_BIT(procnum, 0) ) i += 1;
    ret->iseven = i%2 == 0;

    L4C(log4c_category_debug(GlobLogCat,
                         "mcMPICommContext(): rank %d of %d procs",
                         ret->rank, ret->np));

    return ret;
}

void delMPICommContext( CommContext cntxt )
{
    if ( cntxt->comm != MPI_COMM_NULL )
        MPI_Comm_free(&cntxt->comm);
}
