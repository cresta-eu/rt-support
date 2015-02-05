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

#ifndef _CRBUFFER_INCLUDED
#define _CRBUFFER_INCLUDED

#include "crdef.h"

/* ------------------------------------------------------------------------- */

#ifdef CR_BUFFER_IMPL

struct _cr_buffer
{
    int   buf_size;
    int   bytes;
    char *buf_ptr;
    char *start_ptr;

    int cur_pos; // used only for InBuf
};

#endif // CR_BUFFER_IMPL

int compress( CrBuffer *crBuffer );
int check_buffer( CrBuffer *combuf, CrBuffer *out_buf, int dim, int myprognum );

void sub_buffer( CrBuffer *sub_buf, CrBuffer *host_buf );


/* ------------------------------------------------------------------------- */

#endif // _CRBUFFER_INCLUDED
