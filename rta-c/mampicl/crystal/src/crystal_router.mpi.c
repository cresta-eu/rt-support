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

#include "crystal_router.h"
#include "crystal.h"

#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "crystal_aux.h"

#ifdef USE_LOG4C
static void
log_cr_router( const char *prompt, CrRouter *crr )
{
    log4c_category_debug(GlobLogCat,
                         "%s: CommContext: rank %d of %d procs",
                         prompt, crr->comm->rank, crr->comm->np);
    log4c_category_debug(GlobLogCat,
                         "%s: CrRouter: rank %d of %d procs"
                         " for %d dimenstions",
                         prompt, crr->procnum, crr->nproc, crr->doc);
}
#endif //USE_LOG4C

static int
write_msg_func(CrMsgHeader *msg_hd, int recipient, void *userdata)
{
    CrBuffer *dest_buff = userdata;

    // new mail length (with only one recipient (myself))
    int cpy_size = cr_msg_len(msg_hd, MSG_HEAD|MSG_CONT)
                        + sizeof(cr_msg_recipient(msg_hd, 0));
    // input buffer length including the new mail
    int temp_bytes = dest_buff->bytes + cpy_size;

    if ( temp_bytes <= dest_buff->buf_size ) // it will fit
    {
        // mail header into dest_buf
        CrMsgHeader * const inmsg_hd = (void *)dest_buff->buf_ptr;

        inmsg_hd->ndest    = 1;
        inmsg_hd->src      = msg_hd->src;
        inmsg_hd->nbytes   = msg_hd->nbytes;
        inmsg_hd->status   = msg_hd->status;
        cr_msg_recipient(inmsg_hd, 0) = recipient;

        dest_buff->buf_ptr += cr_msg_len(inmsg_hd, MSG_HEAD|MSG_RECIP);

        // message data into dest_buff
        memcpy(cr_msg_content(inmsg_hd),
               cr_msg_content(msg_hd), msg_hd->nbytes);

        // Update status of dest_buff
        dest_buff->buf_ptr += msg_hd->nbytes;
        dest_buff->bytes    = temp_bytes;

        return 0;
    }

    return -1;
}


static int
copy_to_buf( CrBuffer *dest_buff, CrBuffer *src_buff, int recipient,
             Write_msg_func user_func, void *userdata )
{
    L4C(__log4c_category_trace(GlobLogCat, "copy_to_buf() -- begin"));

    int resp = 0;

    char *buf_ptr = src_buff->start_ptr; // first mail in buffer
    int  bytes;

    for (bytes = 0; bytes < src_buff->bytes; )
    {
        // Read mail header
        CrMsgHeader * const msg_hd = (void *)buf_ptr;
        int message_size = cr_msg_len(msg_hd, MSG_HEAD|MSG_RECIP|MSG_CONT);

        if ( msg_hd->status == OLD ) // Ignore it, process next one
        {
            buf_ptr += message_size;
            bytes   += message_size;
            continue;
        }

        int cur_dest = msg_hd->ndest;                 // number of recipients

        for (int i = 0; i < msg_hd->ndest; ++i)
        {
            int dest = cr_msg_recipient(msg_hd, i);   // next recipient

            if ( dest == recipient)                // you have mail
            {
                int ok;

                if ( user_func != NULL)
                    ok = (*user_func)(msg_hd, recipient, userdata);
                else
                    ok = write_msg_func(msg_hd, recipient, dest_buff);

                if ( ok == 0 )
                {   // take my address from original mail's recipient list.
                    cr_msg_recipient(msg_hd, i) = -99;
                    cur_dest--; // number of remaining recipients in orig. mail
                }
                else // mail does not fit into dest_buff
                    resp = -1;
            }
            else if ( dest < 0 )
                cur_dest--; // already removed from recipients list
        }
        if ( !cur_dest ) // no recipients remaining
            msg_hd->status = OLD;

        // advance to next mail in the buffer
        bytes   += message_size; // update current position
        buf_ptr += message_size;
    }

    L4C(__log4c_category_trace(GlobLogCat, "copy_to_buf() -- end"));
    return resp;
}

int
crystal_router( CrRouter *crr )
{
    L4C(__log4c_category_trace(GlobLogCat, "crystal_router(%x) -- begin", crr));
    L4C(log_buffer_content("crystal_router() -- begin: output buffer",
                       &crr->out_buf, (CURRENT|OLD)));
    L4C(log_buffer_content("crystal_router() -- begin: input buffer",
                       &crr->in_buf, (CURRENT|OLD)));

    int rec_bytes, d, resp, chan, read_bytes;
    CrBuffer combuf;

    resp = 0;
    compress(&crr->in_buf);
    resp += copy_to_buf(&crr->in_buf, &crr->out_buf, crr->procnum,
                        crr->write_msg_func, crr->userdata);

    for (d = 0; d < crr->doc; d++)
    {
        L4C(__log4c_category_trace(GlobLogCat,
                               "  crystal_router():\tdimension %d", d));

        chan = 1<<d;
        if ( d > 0 )
            resp += compress(&crr->out_buf);

        crb_clear(&crr->send_buf);
        if ( crr->read_msg_func != NULL )
        (*crr->read_msg_func)(&crr->send_buf, d, crr->userdata);
        resp += check_buffer(&crr->send_buf, &crr->out_buf, d, crr->procnum);

        sub_buffer(&combuf, &crr->out_buf);

        L4C(log_buffer_content("crystal_router() - send buffer",
                               &crr->send_buf, (CURRENT|OLD)));

        read_bytes = cishift(combuf.start_ptr, combuf.buf_size, chan,
                            crr->send_buf.start_ptr, crr->send_buf.bytes, chan,
                            crr->comm);

        if (read_bytes < 0)
        {
            // TODO: This seems to be wrong ??!!??!!
            rec_bytes = rescue_buf(combuf.start_ptr, combuf.buf_size);
            resp = -1;
        }
        else
            rec_bytes = read_bytes;
        combuf.bytes = rec_bytes;
        combuf.buf_ptr = combuf.start_ptr + rec_bytes;

        resp += copy_to_buf(&crr->in_buf, &combuf, crr->procnum,
                            crr->write_msg_func, crr->userdata);

        // absorb combuf
        crr->out_buf.bytes  += rec_bytes;
        crr->out_buf.buf_ptr = crr->out_buf.start_ptr + crr->out_buf.bytes;
    }
    crr->in_buf.cur_pos = crr->in_buf.bytes;
    compress(&crr->out_buf);

    L4C(log_buffer_content("crystal_router() -- end: output buffer",
                       &crr->out_buf, (CURRENT|OLD)));
    L4C(log_buffer_content("crystal_router() -- end: input buffer",
                       &crr->in_buf, (CURRENT|OLD)));
    L4C(__log4c_category_trace(GlobLogCat, "crystal_router() -- end"));

    return (resp < 0) ? -1 : 0;
}


CrRouter *
new_crystal_router( CommContext comm )
{
    CrRouter *ret = malloc(sizeof(CrRouter));

    ret->comm = comm;
    ret->procnum = comm->rank;
    ret->nproc = comm->np;
    ret->doc = 0;
    for (int i = ret->nproc; i > 1; i = i >> 1)
        ret->doc += 1;

    init_cr_buf(&ret->in_buf, 300*1024*1024);
    init_cr_buf(&ret->out_buf, 300*1024*1024);
    init_cr_buf(&ret->send_buf, 300*1024*1024);

    ret->read_msg_func  = NULL;
    ret->write_msg_func = NULL;
    ret->userdata       = NULL;

    L4C(log_cr_router("new_crystal_router()", ret));
    return ret;
}


void
del_crystal_router( CrRouter *router )
{
    if ( router == NULL )
        return;
    free(router->in_buf.start_ptr);
    free(router->out_buf.start_ptr);
    free(router->send_buf.start_ptr);
    delMPICommContext(router->comm);

}

CrBuffer *crb_buffer( CrRouter *router, int buff_id )
{
    if ( router == NULL )
        return NULL;
    return (buff_id == IN) ? &router->in_buf : &router->out_buf;
}
