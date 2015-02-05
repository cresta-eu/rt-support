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

#ifndef _COMMPRIM_INCLUDED
#define _COMMPRIM_INCLUDED

#ifdef MPI_ALLOWED
#include <mpi.h>
#endif // MPI_ALLOWED

/* ------------------------------------------------------------------------- */

struct _comm_cntxt;
typedef struct _comm_cntxt *CommContext;

int commcntxt_size( CommContext comm );
int commcntxt_id( CommContext comm );

int cwrite( void *buffer, int nbytes, unsigned int chan, CommContext comm );
int ciwrite( void *buffer, int nbytes, unsigned int chan, CommContext comm );

int cread( void *buffer, int nbytes,
           unsigned int inchan, unsigned int outchan, CommContext comm );
int ciread( void *buffer, int nbytes,
            unsigned int inchan, unsigned int outchan, CommContext comm );

int cshift( void *inbuf, int inbytes, unsigned int inchan,
            void *outbuf, int outbytes, unsigned int outchan,
            CommContext comm );
int cishift( void *inbuf, int inbytes, unsigned int inchan,
             void *outbuf, int outbytes, unsigned int outchan,
             CommContext comm );

#ifdef MPI_ALLOWED

struct _comm_cntxt
{
    MPI_Comm comm;
    MPI_Request reqs[20];
    int open_reqs;
    int rank;
    int np;

    int iseven;
};

CommContext mcMPICommContext( MPI_Comm comm );
void delMPICommContext( CommContext cntxt );

#endif // MPI_ALLOWED

/* ------------------------------------------------------------------------- */

#endif // _COMMPRIM_INCLUDED
