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

#define _XOPEN_SOURCE 600

#define CR_BUFFER_IMPL
#include "crbuffer.h"

#include "crystal.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "logging.h"
#include "crystal_aux.h"

/* ------------------------------------------------------------------------- */
/* internal functions */

CrBuffer *
init_cr_buf( CrBuffer *buffer, int size )
{
    // int psize = getpagesize();
    int psize = sysconf(_SC_PAGESIZE);

    buffer->buf_size  = size;
    posix_memalign((void **)&buffer->start_ptr,
                   psize, buffer->buf_size*sizeof(char));
    buffer->buf_ptr   = buffer->start_ptr;
    buffer->bytes     = 0;
    buffer->cur_pos   = 0;

    return buffer;
}


void
sub_buffer( CrBuffer *sub_buf, CrBuffer *host_buf )
{
    // Initialise sub_buf
    sub_buf->start_ptr = host_buf->buf_ptr;
    sub_buf->buf_size  = host_buf->buf_size - host_buf->bytes;
    sub_buf->buf_ptr   = sub_buf->start_ptr;
    sub_buf->bytes     = 0;
}


/**
 * Append a data buffer to a CrBuffer.
 */
int
add_to_buffer( CrBuffer *restrict destbuf, void *restrict src, int nbytes )
{
    int temp_bytes = destbuf->bytes + nbytes;

    if ( temp_bytes > destbuf->buf_size ) // buffer to short
        return -1;

    // copy data into buffer
    memcpy(destbuf->buf_ptr, src, nbytes);

    destbuf->buf_ptr += nbytes;
    destbuf->bytes = temp_bytes;

    return 0;
}


/**
 * Calculate used length of a buffer until at most max_bytes. Mails not fitting
 * into this length are disregarded.
 */
int
rescue_buf( void *buffer, int max_bytes )
{
    int nbytes, ndest, temp, bytes;
    char *buf_ptr;

    buf_ptr = buffer;
    bytes = 0; // Current accepted length
    while ( bytes < max_bytes )
    {
        // Calculate temp := length including next mail
        temp = bytes + 4*sizeof(int);                  // header length
        if ( temp > max_bytes ) break;
        ndest  = *((int *)buf_ptr);              // recipients list
        nbytes = *((int *)buf_ptr + 2*sizeof(int));    // message length
        temp  += nbytes + ndest*sizeof(int);
        if ( temp > max_bytes ) break;

        // Mail fits into max_bytes
        bytes = temp;
        buf_ptr += nbytes + (ndest + 4)*sizeof(int);
    }

    return bytes; // length (< max_bytes) used for mails
}

int
compress( CrBuffer *crBuffer )
{
    L4C(__log4c_category_trace(GlobLogCat, "compress() -- begin"));

    int  old_bytes = crBuffer->bytes;
    char *buf_ptr, *cur_buf_ptr;

    buf_ptr     = crBuffer->start_ptr; // current mail under processing
    cur_buf_ptr = crBuffer->start_ptr; // current insert pos for mails to keep
    int tot_bytes = 0;                 // amount of bytes to keep after compress
    for (int bytes = 0; bytes < crBuffer->bytes; ) // until the end of the buffer
    {
        // read message header
        CrMsgHeader *msg_hd = (CrMsgHeader *)buf_ptr;
        int message_size = cr_msg_len(msg_hd, MSG_HEAD|MSG_RECIP|MSG_CONT);

        if ( msg_hd->status == CURRENT ) // keep the mail
        {
            if ( cur_buf_ptr != buf_ptr ) // copy over old mail
                memmove(cur_buf_ptr, buf_ptr, message_size);
            // else
            //     message already at the right position

            // increment postion pointers iac
            cur_buf_ptr += message_size;
            buf_ptr     += message_size;
            // new buffer size to keep
            tot_bytes += message_size;
        }
        else // old mail, will be deleted.
            buf_ptr += message_size;
        bytes += message_size;
    }
    // update buffer data structure
    crBuffer->bytes   = tot_bytes;
    crBuffer->buf_ptr = cur_buf_ptr;

    L4C(__log4c_category_trace(GlobLogCat, "compress() -- end"));
    return old_bytes - crBuffer->bytes;
}


int
check_buffer( CrBuffer *combuf, CrBuffer *out_buf, int dim, int myprognum )
{
    L4C(__log4c_category_trace(GlobLogCat, "check_buffer() -- begin"));

    int err  = 0;                   // think positive...
    int mask = 1<<dim;              // requested channel
    int bit  = myprognum & mask;    // != 0 if channel bit set in my procnum

    out_buf->buf_ptr = out_buf->start_ptr;    // first mail in out_buf
    int bytes        = 0;

    while ( bytes < out_buf->bytes )
    {
        // message information
        CrMsgHeader * const msg_hd = (CrMsgHeader *)out_buf->buf_ptr;
        const int message_size = cr_msg_len(msg_hd, MSG_HEAD|MSG_RECIP|MSG_CONT);

        if ( msg_hd->status == OLD ) // already processed, goto next one
        {
            out_buf->buf_ptr += message_size;
            bytes            += message_size;
            continue;
        }

        int cur_dest = msg_hd->ndest;   // number of recipients
        int sent     = 0;               // number found recipients that
                                        // will get mail now.

        // current length of send buffer (for restoring)
        const int c_bytes = combuf->bytes;
        // current insert position in send buffer
        CrMsgHeader * const outmsg_hd = (CrMsgHeader *)combuf->buf_ptr;

        for (int i = 0; i < msg_hd->ndest; ++i)
        {
            int dest = cr_msg_recipient(msg_hd, i);

            if ( dest < 0 )   // don't consider already processed recipient
                cur_dest--;
            else if ( (dest & mask)^bit )   // channel bit in dest
            {                               // differs from mine -> send
                if ( !sent )
                {   // first recipient found, copy header to send buffer
                    err += add_to_buffer(combuf, msg_hd,
                                         cr_msg_len(msg_hd, MSG_HEAD));
                    if ( err < 0 ) break;
                }
                // register recipient in send buffer
                err += add_to_buffer(combuf, &dest, sizeof(dest));
                if ( err < 0 ) break;

                // Everything fine, count new recipient
                sent += 1;
                // remove recipient from original mail
                cr_msg_recipient(msg_hd, i) = -99;
                cur_dest--;
            }
        }
        if ( err == 0 && sent ) // add mail content for sending
        {   // copy mail content
            err += add_to_buffer(combuf, cr_msg_content(msg_hd),
                                         cr_msg_len(msg_hd, MSG_CONT));
            if ( err == 0 )
            {
                // update number of recipients in outgoing mail
                outmsg_hd->ndest = sent;
                if ( !cur_dest ) // no remaining recipients in original mail
                    msg_hd->status = OLD;
            }
        }
        if ( err < 0 ) // some problem experienced
        {
            // copy removed recipients back to original mail.
            int *start_ptr = cr_msg_recipient_list(msg_hd);

            for (int i = 0; i < sent; ++i)
            {
                int dest = cr_msg_recipient(outmsg_hd, i);

                while ( *start_ptr != -99 ) start_ptr += 1;
                *start_ptr = dest;
            }
            // reset send buffer
            combuf->bytes   = c_bytes;
            combuf->buf_ptr = (char *)outmsg_hd;
        }
        // advance to next mail in out_buf
        out_buf->buf_ptr += message_size;
        bytes            += message_size;
    }

    L4C(__log4c_category_trace(GlobLogCat, "check_buffer() -- end"));
    return (err < 0) ? -1 : 0;
}

static void
test_buffer_wrap( CrBuffer *buffer )
{
    if ( buffer->cur_pos >= buffer->bytes )
    {
        buffer->cur_pos = 0;
        buffer->buf_ptr = buffer->start_ptr;
    }
}

static void
crb_next_message(CrBuffer *buffer, int msg_size)
{
    buffer->buf_ptr += msg_size;
    buffer->cur_pos += msg_size;
}


/**
 * Get mail from crystal router buffer.
 *
 * @param buffer (I) Buffer to scan for messages.
 * @param msg (I/O) message data.
 *            Input: Message source and destination
 *            Output: Message source and length if found
 * @param msg_txt (O) Message text if message found.
 *
 * @return 0 if message found, error code otherwise
 */
int
get_message( CrBuffer *buffer,
             int msg_dst, int *msg_src, int *msg_len, void *msg_txt )
{
    test_buffer_wrap(buffer);
    if ( buffer->bytes == 0 ) // empty buffer
        return 1;

    int ncur  = 0;          // number of unprocessed messages in buffer
    int bytes = 0;          // number of processed bytes

    while ( bytes < buffer->bytes ) // buffer not completely scanned
    {
        CrMsgHeader *msg_hd = crb_create_msg(buffer);

        int message_size = cr_msg_len(msg_hd, MSG_HEAD|MSG_RECIP|MSG_CONT);

        if ( msg_hd->status == CURRENT )
        {
            ncur++;
            if ( msg_dst == msg_hd->dests[0]
                 && (*msg_src == msg_hd->src || *msg_src == ANY))
            {
                msg_hd->status = OLD;

                *msg_src = msg_hd->src;
                *msg_len = msg_hd->nbytes;
                memcpy(msg_txt, cr_msg_content(msg_hd), msg_hd->nbytes);

                crb_next_message(buffer, message_size);

                return 0;
            }
        }
        bytes += message_size;

        crb_next_message(buffer, message_size),
        test_buffer_wrap(buffer);
    }

    if ( !ncur )
        return 3;

    return 2; // !found
}

int
send_message( CrBuffer *buffer, char *buf, int nbytes, int ndest, int *dest, int from )
{
    if ( nbytes < 0 || ndest < 0 )
        return -1;

    if ( ndest == 0 ) return 0;

    int message_size = sizeof(CrMsgHeader) + (ndest - 1)*sizeof(int) + nbytes;

    if ( buffer->bytes + message_size <= buffer->buf_size )
    {
        CrMsgHeader *msg_hd = crb_create_msg(buffer);

        msg_hd->src = from;
        msg_hd->status = CURRENT;
        crb_add_recipients(msg_hd, dest, ndest);
        crb_add_msgcontent(msg_hd, buf, nbytes);
        crb_close_msg(buffer, msg_hd);

        return 0;
    }
    else
        return -1;
}


#ifdef USE_LOG4C
void
log_buffer_content( const char *prompt,
                    CrBuffer *crBuffer, int msg_status_flags )
{
    __log4c_category_trace(GlobLogCat, "-------------------");
    __log4c_category_trace(GlobLogCat, "BEGIN -- BUFFER DUMP: %s", prompt);

    char *buf_ptr = crBuffer->start_ptr; // current mail under processing
    int bytes;                           // amount of processed bytes of buffer

    for (bytes = 0; bytes < crBuffer->bytes; )
    {
        // read mail header
        const CrMsgHeader *msg_hd;
        int status;

        msg_hd = cr_msg_header(NULL, NULL, NULL, &status, buf_ptr);
        int message_size = cr_msg_len(msg_hd, MSG_HEAD|MSG_RECIP|MSG_CONT);

        if ( (status & msg_status_flags) != 0 )
        {
            __log4c_category_trace(GlobLogCat, "\tmail: %s",
                                   message_string(buf_ptr, 0));
        }
        buf_ptr += message_size;
        bytes   += message_size;
    }

    __log4c_category_trace(GlobLogCat, "END -- BUFFER DUMP: %s", prompt);
    __log4c_category_trace(GlobLogCat, "-------------------");
}
#endif //USE_LOG4C

void
print_buffer(const char *prompt, CrBuffer *buffer)
{
    printf("%s\n", prompt);
    printf("Status Buffer: %p\n", (void *)buffer);
    printf("------------------------------\n");
    printf(" Buffer size:       %d\n", buffer->buf_size);
    printf(" Stored bytes:      %d\n", buffer->bytes);
    printf(" Buffer start:      %p\n", buffer->start_ptr);
    printf(" Buffer insert pos: %p\n", buffer->buf_ptr);
    printf("------------------------------\n");
}

/* ------------------------------------------------------------------------- */
/* public functions */

int
crb_data_len( CrBuffer *buffer )
{
    return buffer != NULL ? buffer->bytes : 0;
}


char *
crb_data( CrBuffer *buffer )
{
    return buffer != NULL ? buffer->start_ptr : NULL;
}



int
crb_clear( CrBuffer *buffer )
{
    if ( buffer != NULL )
    {
        buffer->buf_ptr = buffer->start_ptr;
        buffer->bytes   = 0;
        buffer->cur_pos = 0;
    }
    return 0;
}

/**
 * Resizes a router buffer
 */
int
crb_resize( CrBuffer *buffer, int buf_size )
{
    int resp = 0;

    char *new_ptr = malloc(buf_size);

    if ( new_ptr == NULL )
        return -1;
    if ( buffer->bytes > buf_size )
    {
        resp = -1;

        buffer->bytes = rescue_buf(buffer->start_ptr, buf_size);
        if ( buffer->cur_pos > buf_size )
            buffer->cur_pos = 0;

    }
    char *data_start = buffer->start_ptr;

    buffer->buf_ptr = new_ptr;
    for (int i = 0; i < buffer->bytes; ++i)
        *(buffer->buf_ptr)++ = *data_start++;

    free(buffer->start_ptr);
    buffer->buf_size = buf_size;
    buffer->start_ptr = new_ptr;

    return resp;
}


CrMsgHeader *crb_create_msg( CrBuffer *crb )
{
    return (void *)crb->buf_ptr;
}


void crb_close_msg( CrBuffer *crb, CrMsgHeader *msg )
{
    int len = cr_msg_len(msg, (MSG_HEAD|MSG_RECIP|MSG_CONT));

    crb->buf_ptr += len;
    crb->bytes   += len;
}


void crb_add_recipient( CrMsgHeader *msg, int recipient )
{
    msg->dests[msg->ndest++] = recipient;
}


void crb_add_recipients( CrMsgHeader *msg, const int *recipients, int n )
{
    for (int i = 0; i < n; i++)
        msg->dests[msg->ndest++] = recipients[i];
}

void crb_add_msgcontent( CrMsgHeader *msg, const void *buffer, int n )
{
    msg->nbytes = n;
    memcpy(cr_msg_content(msg), buffer, n);
}
