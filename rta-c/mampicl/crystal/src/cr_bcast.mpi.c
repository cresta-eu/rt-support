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

#include "cr_internal.h"
#include "crystal_router.h"
#include "crystal_aux.h"
#include "logging.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

int
Cr_Bcast(void *buff, int count, MPI_Datatype dtype,
          int root, MPI_Comm comm)
{
    L4C(log4c_category_debug(GlobLogCat,
                         "Cr_Bcast: of %d bytes from root %d\n", count, root));

    int myvrank, cursize;

    myvrank = Crr->comm->rank^root;
    // mask = POW_2(HC_Opts.hcube_d) - 1;
    cursize = sizeof(char); // MPI_DOUBLE

    // Message into out buffer
    crb_clear(&Crr->out_buf);
    if ( myvrank == 0 )
    {
        CrMsgHeader *msg  = crb_create_msg(&Crr->out_buf);

        msg->ndest  = 0;
        msg->src    = myvrank;
        msg->nbytes = cursize*count;
        msg->status = CURRENT;
        for (int i = 0; i < Crr->comm->np; i++)
            msg->dests[msg->ndest++] = i;
        crb_add_msgcontent(msg, buff, msg->nbytes);
        crb_close_msg(&Crr->out_buf, msg);
    }
    crb_clear(&Crr->in_buf);

    crystal_router(Crr);

    // Received message into parameter buff
    if ( myvrank != 0 )
    {
        int src = ANY, nrecvd;

        get_message(&Crr->in_buf, Crr->procnum, &src, &nrecvd, buff);
    }
    crb_clear(&Crr->in_buf);

    return 0;
}
