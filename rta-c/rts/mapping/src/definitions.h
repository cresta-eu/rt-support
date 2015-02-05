/*
 * rta-c - runtime system administration component.
 *  Copyright (C) 2014  Sebastian Tunstig.
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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/* UNCOMMENT FOR EXCESSIVE DEBUG OUTPUT TO STDOUT */
/* #define DEBUG */

#define MPI_TAG 33 /* Used to give a unique message-tagging in MPI */

#define UNSET -1

/* OPTIONS FOR DISTANCE MEASUREMENTS. */
#define MEASUREMENT_ITERATIONS 100 /* Number of send/recv per test  */
#define MEASUREMENT_CLOCK_MULTIPLIER 1000000 /* Used to pad cpu-clock time from doubles to integer values. */

/* SCOTCH STRATEGY STRINGS */
#define STRATEGY_BIPARTITIONING "f{move=1000}/((load0=load)|(load0=0))?x;"
#define STRATEGY_REFINEMENT "f{move=100,pass=-1,bal=0.01}"
//#define STRATEGY_STRING "m{asc=b{width=3,bnd=d{pass=40,dif=1,rem=0}f{move=80,pass=-1,bal=0.01},org=f{move=80,pass=-1,bal=0.01}},low=r{job=t,bal=0.01,map=t,poli=S,sep=(m{asc=b{bnd=f{move=120,pass=-1,bal=0.01,type=b},org=f{move=120,pass=-1,bal=0.01,type=b},width=3},low=h{pass=10}f{move=120,pass=-1,bal=0.01,type=b},vert=120,rat=0.8}|m{asc=b{bnd=f{move=120,pass=-1,bal=0.01,type=b},org=f{move=120,pass=-1,bal=0.01,type=b},width=3},low=h{pass=10}f{move=120,pass=-1,bal=0.01,type=b},vert=120,rat=0.8})},vert=10000,rat=0.8,type=0}"

#define STRATEGY_STRING "m{asc=b{width=3,bnd=d{pass=40,dif=1,rem=0}f{move=80,pass=-1,bal=0},org=f{move=80,pass=-1,bal=0}},low=r{job=t,bal=0,map=t,poli=S,sep=(m{asc=b{bnd=f{move=120,pass=-1,bal=0,type=b},org=f{move=120,pass=-1,bal=0,type=b},width=3},low=h{pass=10}f{move=120,pass=-1,bal=0,type=b},vert=120,rat=0.8}|m{asc=b{bnd=f{move=120,pass=-1,bal=0,type=b},org=f{move=120,pass=-1,bal=0,type=b},width=3},low=h{pass=10}f{move=120,pass=-1,bal=0,type=b},vert=120,rat=0.8})},vert=10000,rat=0.8,type=0}"
#endif

