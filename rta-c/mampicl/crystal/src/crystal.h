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

#ifndef _CRYSTAL_INTERNAL_INCLUDED
#define _CRYSTAL_INTERNAL_INCLUDED

#include "crdef.h"

/* ------------------------------------------------------------------------- */

CrRouter *new_crystal_router( CommContext comm );
void      del_crystal_router( CrRouter *router );

int crystal_router( CrRouter *cr_router );
int crystal_router2( CrRouter *cr_router );

// TODO: parameter msg_txt as void *
int get_message( CrBuffer *buffer,
                 int msg_dst, int *msg_src, int *msg_len, void *msg_txt );
int send_message( CrBuffer *buffer,
                  char *buf, int nbytes, int ndest, int *dest, int from );


CrMsgHeader *crb_create_msg( CrBuffer *crb );
void crb_close_msg( CrBuffer *crb, CrMsgHeader *msg );
void crb_add_recipient( CrMsgHeader *msg, int recipient );
void crb_add_recipients( CrMsgHeader *msg, const int *recipients, int n );
void crb_add_msgcontent( CrMsgHeader *msg, const void *buffer, int n );


int crb_resize( CrBuffer *buffer, int buf_size );

CrBuffer *crb_buffer( CrRouter *router, int buff_id );

#define IN         0
#define OUT        1

int crb_data_len( CrBuffer *buffer );
char *crb_data( CrBuffer *buffer );
int crb_clear( CrBuffer *buffer );

/* ------------------------------------------------------------------------- */

#endif // _CRYSTAL_INTERNAL_INCLUDED
