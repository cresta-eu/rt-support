/*
 * rta-c - runtime system administration component.
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

#include "time_measurement.h"

#define _GNU_SOURCE

#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/* ------------------------------------------------------------------------- */

#ifndef __MACH__
static clockid_t clock_type;
#else
static clock_serv_t clock_type;
#endif


int init_time_measurements( void )
{
#ifndef __MACH__
#define NR_CLOCKS 2
    clockid_t try_clocks[] = { CLOCK_MONOTONIC, CLOCK_REALTIME };
    struct timespec tp;

    for (int i = 0; i < NR_CLOCKS; i++)
        if (clock_getres(try_clocks[i], &tp) == 0)
        {
            clock_type = try_clocks[i];
            return 0;
        }
    return 1;
#undef NR_CLOCKS
#else /* __MACH__ */
    host_get_clock_service(mach_host_self(), REALTIME_CLOCK, &clock_type);
        /* or REALTIME_CLOCK */
    return 0;
#endif
}

void deinit_time_measurements( void )
{
#ifdef __MACH__
    mach_port_deallocate(mach_task_self(), clock_type);
#endif
}


int clock_get_usec(double *usec)
{
#ifndef __MACH__
    struct timespec tp;
    int ret = clock_gettime(clock_type, &tp);

    *usec = 1.0E6*tp.tv_sec + 1.0E-3*tp.tv_nsec;
    return ret;
#else
    mach_timespec_t tp;
    clock_get_time(clock_type, &tp);
    *usec = 1.0E6*tp.tv_sec + 1.0E-3*tp.tv_nsec;
    return 0; /* Assume only the best */
#endif
}
