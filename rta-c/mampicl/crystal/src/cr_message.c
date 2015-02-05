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

#include "crystal_aux.h"

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

/**
 *
 * ATTN.: not thread-safe.
 */
char *
message_string( const void *msg_buf, int with_content )
{
    static char message[1024];
    char tmpstr[100];
    int i, j;

    const CrMsgHeader *msg_hd;
    // int ndest, src, nbytes, status;

    msg_hd = msg_buf;

    *message = '\0';
    sprintf(tmpstr, "ndest: %d", msg_hd->ndest);
    strcat(message, tmpstr);
    sprintf(tmpstr, "\tsrc: %d", msg_hd->src);
    strcat(message, tmpstr);
    sprintf(tmpstr, "\tlength: %d", msg_hd->nbytes);
    strcat(message, tmpstr);
    sprintf(tmpstr, "\tstatus: %s", msg_hd->status == CURRENT ? "CURRENT" : "OLD");
    strcat(message, tmpstr);
    strcat(message, "\tdest:");
    for (i = 0; i < msg_hd->ndest; i++)
    {
        sprintf(tmpstr, " %d", cr_msg_recipient_list(msg_hd)[i]);
        strcat(message, tmpstr);
    }
    strcat(message, " content: ");

    if ( msg_hd->nbytes > 0 )
    {
        if ( with_content != 0 )
        {
            j = strlen(message);
            memcpy(message + j, cr_msg_content(msg_hd), msg_hd->nbytes);
            message[j + msg_hd->nbytes] = '\0';
        }
        else
            strcat(message, "...");
    }
    else
        strcat(message, "-");

    return message;
}


const CrMsgHeader*
cr_msg_header( int *ndest, int *src, int *nbytes, int *status,
                const void *buffer )
{
    const CrMsgHeader *ret = buffer;

    if ( ret != 0 )
    {
        if ( ndest  != NULL ) *ndest  = ret->ndest;
        if ( src    != NULL ) *src    = ret->src;
        if ( nbytes != NULL ) *nbytes = ret->nbytes;
        if ( status != NULL ) *status = ret->status;
    }
    return ret;
}

int
cr_msg_len( const CrMsgHeader *header, int flags )
{
    int ret = 0;

    if ( header != NULL )
    {
        if ( (flags & MSG_HEAD) != 0 )
            ret += sizeof(CrMsgHeader) - sizeof(header->dests[0]);
        if ( (flags & MSG_RECIP) != 0 )
            ret += header->ndest*sizeof(header->dests[0]);
        if ( (flags & MSG_CONT) != 0 )
            ret += header->nbytes;
    }
    return ret;
}
