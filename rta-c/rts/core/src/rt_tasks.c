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

#include "rt_tasks.h"

#include "defines.h"
#include "logging.h"

#if defined(MPE_TRACE)
#include <mpe.h>

#elif defined(PARAVER_TRACE)

#include <stdio.h>
//#include "extrae_user_events.h"

#else
    #define NULL_TRACE 1
    /* nothing */
#endif


#if defined(NULL_TRACE)

void rt_registerTask(TaskId *taskBegin, TaskId *taskEnd, const char *taskName,
        const char *color) { }

void rt_taskBegin(TaskId id, const char *data) { }

void rt_taskEnd(TaskId id, const char *data) { }

void finalizeRTInitialization() { }

#endif /* NULL_TRACE */

#if defined(MPE_TRACE)

void rt_registerTask(TaskId *taskBegin, TaskId *taskEnd, const char *taskName,
        const char *color)
{
    MPE_Log_get_state_eventIDs(taskBegin, taskEnd);
    MPE_Describe_info_state(*taskBegin, *taskEnd, taskName, color, 0 /*"%s"*/);
}

void finalizeRTInitialization()
{

}

void rt_taskBegin(TaskId id, const char *data)
{
    // TODO: Crash if data are used.
    MPE_Log_event(id, 0, 0);
}

void rt_taskEnd(TaskId id, const char *data)
{
    // TODO: Crash if data are used.
    MPE_Log_event(id, 0, 0);
}

#endif /* MPE_TRACE */

#if defined(PARAVER_TRACE)

static TaskId NextEventID = 1;
static int EventTypeID = 10000;

/* Ugly, If you like make it with dynamic data structures.*/
char *EventNames[200];

#define EVENT_ID_FILE "md_taskid.dat"


void rt_registerTask(TaskId *taskBegin, TaskId *taskEnd, const char *taskName,
        const char *colorName)
{
    if ( NextEventID < 0)
    {
        log_error("Tyv?rr f?r sent att registrera h?ndelser!");
        return;
    }
    EventNames[NextEventID] = strsave(taskName);
    *taskBegin = NextEventID++;
}

void finalizeRTInitialization()
{
    /* Format:
    EVENT_TYPE
    9   10000       Task
    VALUES
    1    <name>...
    2    <name>...
    <n+1> End
    */
    FILE *fp = fopen(EVENT_ID_FILE, "wt");

    fprintf(fp, "EVENT_TYPE\n");
    fprintf(fp, "%d\t%d\t%s\n", NextEventID+1, EventTypeID, "Task");

    fprintf(fp, "VALUES\n");
    for (int i = 1; i < NextEventID; i++)
        fprintf(fp, "%d\t%s\n", i, EventNames[i]);
    fprintf(fp, "%d\t%s\n", NextEventID, "End");
    fclose(fp);

    NextEventID = -1; // Nu ?r slut.
}

void rt_taskBegin(TaskId id, const char *data)
{
    Extrae_eventandcounters(10000, id);
}

void rt_taskEnd(TaskId id, const char *data)
{
    Extrae_eventandcounters(10000, 0);
}

#endif /* PARAVER_TRACE */
