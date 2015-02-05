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

/* Implementation of open_memstream() for BSD after a draft from
 * (c) 2005 Richard Kettlewell.
 * Source: http://android-wifi-tether.googlecode.com/svn-history/r411/
 *                                       tools/vde/src/common/open_memstream.c
 * A draft for fmemopen was
 * http://people.freebsd.org/~hmp/patches_old/fmemopen.c.patch
 */

#if defined(__linux__)
#define _GNU_SOURCE 1
#endif

#include "rt_systools.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <stdio.h>
#include <string.h>

#include "defines.h"

/* ------------------------------------------------------------------------- */

#if defined(__APPLE__)

#define REALLOC_JUNK_SIZE   (10*1024)

struct memstream {
    char *buffer;
    size_t dataLen; // amount of data in buffer
    size_t bufferSize; // buffer dataLen
    size_t curPos;

    char **userPtr; // user-owned buffer pointer
    size_t *userSize; // user-known buffer dataLen (== used data amount)

};

static int memstream_readfn(void *u, char *buffer, int len)
{
    struct memstream *m = (struct memstream *) u;

    if (m == NULL)
    {
        errno = EBADF;
        return -1;
    }

    if (m->curPos + len > m->dataLen)
    {
        if (m->curPos == m->dataLen) return -1; // EOF
        len = m->dataLen - m->curPos;
    }

    memcpy(buffer, m->buffer + m->curPos, len);

    m->curPos += len;

    return len;
}

static int memstream_writefn(void *u, const char *buffer, int bytes)
{
    /* No flush is implemented. Therefore append always a null byte to the
     * written data. Also userPtr and userSize are updated by every call.
     *
     * Buffer increase always in junks of 10 kByte to reduce the number
     * of reallocations.
     */
    struct memstream *m = (struct memstream *)u;
    int ret = 0;

    assert(m != NULL);
    assert(bytes >= 0);
    if (m != NULL)
    {
        if (bytes > 0)
        {
            size_t reqDataLen = m->dataLen + bytes + 1;

            if (reqDataLen > m->bufferSize) // enlarge buffer.
            {
                size_t newSize = (reqDataLen / REALLOC_JUNK_SIZE + 1)
                        *REALLOC_JUNK_SIZE;

                char *newBuffer = (char *)realloc(m->buffer, newSize);

                if (newBuffer == NULL)
                    return -1;

                m->buffer = newBuffer;
                m->bufferSize = newSize;

                *m->userPtr = m->buffer;
            }
            memcpy(m->buffer + m->dataLen, buffer, bytes);
            m->dataLen += bytes;
            m->curPos = m->dataLen;
            m->buffer[m->dataLen] = 0;

            *m->userSize = m->dataLen;
            ret = bytes;
        }
    }
    return ret;
}

static int memstream_closefn(void *u)
{
//    struct memstream *m = (struct memstream *)u;
//
//    if (m->buffer != NULL)
//    {
//        free(m->buffer);
//        m->buffer = 0;
//    }

    return 0;
}


#endif

FILE *rt_open_memstream(char **buffer, size_t *bufferSize)
{
    FILE *ret = NULL;

#if defined(__APPLE__)
    struct memstream *m = NULL;

    m = MALLOC(struct memstream);
    ZEROMEM(m, struct memstream);

    if (m != NULL)
    {
        m->buffer = NULL;
        m->bufferSize = 0;
        m->dataLen = 0;
        m->curPos = 0;

        m->userPtr = buffer; // reference to user-owned data
        *m->userPtr = m->buffer; // value to user
        m->userSize = bufferSize; // reference to user-owned data
        *m->userSize = m->dataLen; // value to user

        ret = funopen(m, memstream_readfn, memstream_writefn, 0,
                memstream_closefn);
    }


#endif
#if defined(__linux__)
    ret = open_memstream(buffer, bufferSize);
#endif

    return ret;
}

// At the moment only useful for read operations!!!
FILE * rt_fmemopen(void *buffer, size_t bufferSize, const char *mode)
{
    FILE *ret = NULL;

#if defined(__APPLE__)
    struct memstream *m = NULL;

    m = MALLOC(struct memstream);
    ZEROMEM(m, struct memstream);

    if (m != NULL)
    {
        m->buffer = (char *)buffer;
        m->bufferSize = bufferSize;
        m->dataLen = bufferSize;
        m->curPos = 0;

        // user-owned data are the stream structure itself.
        // should work in write actions, not tested!
        m->userPtr = &m->buffer;
        m->userSize = &m->dataLen;

        ret = funopen(m, memstream_readfn, memstream_writefn, 0,
                memstream_closefn);
    }
#endif
#if defined(__linux__)
    ret = fmemopen(buffer, bufferSize, mode);
#endif

    return ret;
}

#ifdef TEST_MAIN

static char *str_const = "Das ist mein Testtext.";

int main(int argc, char *argv[])
{
    FILE *fp;
    char *buff = NULL;
    size_t buffSize = 0;

    fp = rt_open_memstream(&buff, &buffSize);
    fwrite(str_const, 1, strlen(str_const), fp);
    fclose(fp);

    char readStr[100];

    fp = rt_fmemopen(buff, buffSize, "rb");
    fread(readStr, 1, strlen(str_const), fp);
    fclose(fp);

    readStr[strlen(str_const)] = 0;
    printf("%s\n", buff);

    return 0;
}

#endif /* TEST_MAIN */
