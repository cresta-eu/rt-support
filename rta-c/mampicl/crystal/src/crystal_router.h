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

#ifndef CRYSTAL_ROUTER_INCLUDED
#define CRYSTAL_ROUTER_INCLUDED

#define CR_BUFFER_IMPL
#include "crbuffer.h"

/* ------------------------------------------------------------------------- */

typedef void (*Read_msg_func)(CrBuffer *buff, int dim, void *userdata);
typedef int (*Write_msg_func)(CrMsgHeader *msg_hd, int dest, void *userdata);

struct _cr_router
{
    CommContext comm;
    int         procnum;
    int         nproc;
    int         doc;
    CrBuffer    in_buf;
    CrBuffer    out_buf;
    CrBuffer    send_buf;

    Read_msg_func  read_msg_func;
    Write_msg_func write_msg_func;

    void       *userdata;

};

/* ------------------------------------------------------------------------- */

#endif // CRYSTAL_ROUTER_INCLUDED
