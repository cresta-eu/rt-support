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

#ifndef _CRYSTAL_AUX_INCLUDED
#define _CRYSTAL_AUX_INCLUDED

#include "crdef.h"

/* ------------------------------------------------------------------------- */
/* message buffer */

CrBuffer *init_cr_buf( CrBuffer *buffer, int size );
int add_to_buffer( CrBuffer *destbuf, void *src, int nbytes );
int rescue_buf( void *buffer, int max_bytes );

#ifdef USE_LOG4C
void log_buffer_content( const char *prompt,
                         CrBuffer *crBuffer, int msg_status_flags );
#endif // USE_LOG4C

void print_buffer(const char *prompt, CrBuffer *buffer);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* router internal message implementation */

struct _crmsg_header
{
    int ndest;      // number of recipients
    int src;        // sender
    int nbytes;     // length of the message
    int status;     // status: OLD or CURRENT
    int dests[1];   // recipient list

};

// flags to specify the message status
#define CURRENT    1
#define OLD        2

// flags to specify parts of the message
#define MSG_HEAD  1 // header information
#define MSG_RECIP 2 // recipient list
#define MSG_CONT  4 // content

const CrMsgHeader* cr_msg_header( int *ndest, int *src, int *nbytes, int *status,
                                  const void *buffer );
int cr_msg_len( const CrMsgHeader *header, int flags );

#define cr_msg_recipient_list(pMsg) ((pMsg)->dests)
#define cr_msg_recipient(pMsg, i)   (((pMsg)->dests)[i])
#define cr_msg_content(pMsg)        ((void *)((pMsg)->dests + (pMsg)->ndest))

char *message_string( const void *msg_buf, int with_content );

/* ------------------------------------------------------------------------- */

#endif // _CRYSTAL_AUX_INCLUDED
