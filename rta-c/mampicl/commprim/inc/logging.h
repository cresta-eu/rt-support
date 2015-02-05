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

#ifndef _LOGGING_INCLUDED
#define _LOGGING_INCLUDED

#ifdef USE_LOG4C

#include <log4c.h>
#include <log4c/defs.h>
#include <log4c/appender.h>
#include <log4c/appender_type_stream.h>
#include <log4c/priority.h>
#include <log4c/location_info.h>

#endif //USE_LOG4C

#ifndef LOGGING_EXT
#define LOGGING_EXT extern
#endif


/* ------------------------------------------------------------------------- */

#ifdef USE_LOG4C

LOGGING_EXT log4c_category_t *GlobLogCat;

void log_init(int world_rank);
void log_fini( void );

#define L4C(call)  call
#define SIMPLETRACE(msg) __log4c_category_trace(GlobLogCat, (msg))

#else

#define L4C(call)
#define SIMPLETRACE(msg)

#define log_init(arg1)
#define log_fini(arg1)

#define XSTR(s) STR(s)
#define STR(s) #s

#endif // USE_LOG4C

/* ------------------------------------------------------------------------- */

#endif // _LOGGING_INCLUDED
