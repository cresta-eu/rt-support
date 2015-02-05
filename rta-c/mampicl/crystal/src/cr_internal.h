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

#ifndef _CR_INTERNAL_INCLUDED
#define _CR_INTERNAL_INCLUDED

#include <mpi.h>

#include "crystal.h"

/* ------------------------------------------------------------------------- */

#ifndef CR_INTERAL_EXTDEF
#define CR_INTERAL_EXTDEF extern
#endif

CR_INTERAL_EXTDEF CrRouter *Crr;

/* ------------------------------------------------------------------------- */

#endif // _CR_INTERNAL_INCLUDED
