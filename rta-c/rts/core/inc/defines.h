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

#ifndef DEFINES_H_
#define DEFINES_H_

#include <memory.h>
#include <stdlib.h>

/*--------------------------------------------------------------------------*/

#define MALLOC(t)       MALLOCN(t, 1)
#define MALLOCN(t, n)   ((t *)malloc((n)*sizeof(t)))

#define ZEROMEM(p, t)       ZEROMEMN(p, 1, t)
#define ZEROMEMN(p, n, t)   memset((p), 0, (n)*sizeof(t))

#define strsave(s) strcpy(MALLOCN(char, strlen((s))+1), (s))

/*--------------------------------------------------------------------------*/

#endif /* DEFINES_H_ */
