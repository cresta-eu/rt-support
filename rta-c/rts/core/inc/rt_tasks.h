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

#ifndef RT_TASKS_H_
#define RT_TASKS_H_

/*---------------------------------------------------------------------------*/

typedef int TaskId;
typedef void *rt_task;

void rt_registerTask(TaskId *taskBegin, TaskId *taskEnd, const char *taskName,
        const char *name);
void finalizeRTInitialization();

void rt_taskBegin(TaskId id, const char *data);
void rt_taskEnd(TaskId id, const char *data);

// Tag for RT internal MPI communication
#define RT_INTERNAL_MPI 999

/*---------------------------------------------------------------------------*/

#endif /* RT_TASKS_H_ */
