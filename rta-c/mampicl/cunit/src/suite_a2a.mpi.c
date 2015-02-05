

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

#include "cunit_mpi.h"

#include "cr_mpi.h"
#include "logging.h"

/* ------------------------------------------------------------------------- */

typedef int (*Alltoallv_func)(
    MPI_CPARM void *sendbuf, MPI_CPARM int *sendcounts, MPI_CPARM int *sdispls,
        MPI_Datatype sendtype,
    void *recvbuf, MPI_CPARM int *recvcounts, MPI_CPARM int *rdispls,
        MPI_Datatype recvtype,
    MPI_Comm comm);

typedef int (*Alltoall_func)(
    MPI_CPARM void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype,
    MPI_Comm comm);

/* ------------------------------------------------------------------------- */

/*
 * Suite name : Alltoall_MPI
 */
/* ------------------------------------------------------------------------- */

static int numprocs, myrank;

// Forward declarations of tests.
DECL_TESTFUNC(cr_alltoallv);
DECL_TESTFUNC(cr_alltoallv_in_place);
DECL_TESTFUNC(cr_alltoallv2);
DECL_TESTFUNC(cr_alltoallv2_in_place);
DECL_TESTFUNC(cr_alltoall);
DECL_TESTFUNC(mpi_alltoallv);
DECL_TESTFUNC(mpi_alltoallv_in_place);
DECL_TESTFUNC(mpi_alltoall);


// Definition of the test suite
CU_TestInfo tests_mpi_coll[] = {
    CU_TESTINFO(cr_alltoallv),
    CU_TESTINFO(cr_alltoallv_in_place),
    CU_TESTINFO(cr_alltoallv2),
    CU_TESTINFO(cr_alltoallv2_in_place),
    CU_TESTINFO(cr_alltoall),
    CU_TESTINFO(mpi_alltoallv),
    CU_TESTINFO(mpi_alltoallv_in_place),
    CU_TESTINFO(mpi_alltoall),
    CU_TEST_INFO_NULL
};

/* ------------------------------------------------------------------------- */
/* Suite management */

int init_suite_mpi_coll( void )
{
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    Cr_Init(MPI_COMM_WORLD);

    return 0;
}

int cleanup_suite_mpi_coll( void )
{
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Test implementations */

static void *alloc_buffer(int len)
{
    return memset(malloc(len), 0, len);
}

static void
test_alltoallv_impl( Alltoallv_func test_func )
{
    double *sendbuf, *recvbuf, *backbuf;
    int *scount, *rcount;
    int *sdispls, *rdispls;
    int max_len = 1000;
    int buf_len = numprocs*max_len*sizeof(double);
    int seed = 8482;

    sendbuf = alloc_buffer(buf_len*sizeof(double));
    recvbuf = alloc_buffer(buf_len*sizeof(double));
    backbuf = alloc_buffer(buf_len*sizeof(double));
    scount  = alloc_buffer(numprocs*sizeof(int));
    rcount  = alloc_buffer(numprocs*sizeof(int));
    sdispls = alloc_buffer((numprocs+1)*sizeof(int));
    rdispls = alloc_buffer((numprocs+1)*sizeof(int));
    // create an individual seed value for every rank.
    srand(seed);
    for (int i = 0; i < myrank + 1; i++)
        seed = rand();
    // initialize the random sequence of the rank.
    srand(seed);
    // calculate the number of elements to send to each process (at most max_len)
    sdispls[0] = 0;
    for (int i = 0; i < numprocs; i++)
    {
        scount[i] = (((double)rand())/RAND_MAX)*max_len;
        sdispls[i+1] = sdispls[i] + scount[i];
    }
    // we learn now how much we have to receive.
    MPI_Alltoall(scount, 1, MPI_INT, rcount, 1, MPI_INT, MPI_COMM_WORLD);
    rdispls[0] = 0;
    for (int i = 0; i < numprocs; i++)
        rdispls[i+1] = rdispls[i] + rcount[i];

    // fill the send buffer (each rank gets an individual content)
    for (int i = 0; i < sdispls[numprocs]; i++)
        sendbuf[i] = rand();

    // send data to all receivers
    (*test_func)(sendbuf, scount, sdispls, MPI_DOUBLE,
                 recvbuf, rcount, rdispls, MPI_DOUBLE, MPI_COMM_WORLD);
    // send it back to the senders
    (*test_func)(recvbuf, rcount, rdispls, MPI_DOUBLE,
                 backbuf, scount, sdispls, MPI_DOUBLE, MPI_COMM_WORLD);

    // now must be valid *sendbuf == *backbuf
    CU_TEST(memcmp(sendbuf, backbuf, sdispls[numprocs]*sizeof(double)) == 0);

    free(sendbuf);
    free(recvbuf);
    free(backbuf);
    free(scount);
    free(rcount);
    free(sdispls);
    free(rdispls);
}


static void
test_alltoallv_in_place_impl( Alltoallv_func test_func )
{
    double *sendbuf, *recvbuf;
    int *scount, *rcount;
    int *sdispls;
    int max_len = 1000;
    int buf_len = numprocs*max_len*sizeof(double);
    int seed = 8482;

    sendbuf = alloc_buffer(buf_len*sizeof(double));
    recvbuf = alloc_buffer(buf_len*sizeof(double));
    scount  = alloc_buffer(numprocs*sizeof(int));
    rcount  = alloc_buffer(numprocs*sizeof(int));
    sdispls = alloc_buffer((numprocs+1)*sizeof(int));
    // create an individual seed value for every rank.
    srand(seed);
    for (int i = 0; i < myrank + 1; i++)
        seed = rand();
    // initialize the random sequence of the rank.
    srand(seed);
    // number of elements to send to each process (at most max_len)
    for (int i = 0; i < numprocs; i++)
        if (i >= myrank)
            scount[i] = (((double)rand())/RAND_MAX)*max_len;
        else
            scount[i] = 0;
    // we learn now how much we have to receive.
    MPI_Alltoall(scount, 1, MPI_INT, rcount, 1, MPI_INT, MPI_COMM_WORLD);
    // complete the send and receive counts
    for (int i = 0; i < myrank; i++)
        scount[i] = rcount[i];
    sdispls[0] = 0;
    for (int i = 0; i < numprocs; i++)
        sdispls[i+1] = sdispls[i] + scount[i];

    // fill the send buffer (each rank gets an individual content)
    for (int i = 0; i < sdispls[numprocs]; i++)
        sendbuf[i] = rand();

    // send data to all receivers
    (*test_func)(sendbuf, scount, sdispls, MPI_DOUBLE,
                 recvbuf, scount, sdispls, MPI_DOUBLE, MPI_COMM_WORLD);
    // send it back to the senders
    (*test_func)(MPI_IN_PLACE, NULL, NULL, MPI_DOUBLE,
                 recvbuf, scount, sdispls, MPI_DOUBLE, MPI_COMM_WORLD);

    // now must be valid *sendbuf == *recvbuf
    CU_TEST(memcmp(sendbuf, recvbuf, sdispls[numprocs]*sizeof(double)) == 0);

    free(sendbuf);
    free(recvbuf);
    free(scount);
    free(rcount);
    free(sdispls);
}

static void
test_alltoall_impl( Alltoall_func test_func)
{
    double *sendbuf, *recvbuf, *backbuf;
    int sendcount, recvcount, backcount;
    int max_len = 1000;
    int buf_len = numprocs*max_len*sizeof(double);
    int seed = 8482;

    sendbuf = alloc_buffer(buf_len*sizeof(double));
    recvbuf = alloc_buffer(buf_len*sizeof(double));
    backbuf = alloc_buffer(buf_len*sizeof(double));

    // create an individual seed value for every rank.
    srand(seed);
    for (int i = 0; i < myrank + 1; i++)
        seed = rand();
    // initialize the random sequence of the rank.
    srand(seed);
    // fill the send buffer (each rank gets an individual content)
    for (int i = 0; i < numprocs*max_len; i++)
        sendbuf[i] = rand();

    sendcount = recvcount = backcount = max_len;
    // send data to all receivers
    (*test_func)(sendbuf, sendcount, MPI_DOUBLE,
                 recvbuf, recvcount, MPI_DOUBLE, MPI_COMM_WORLD);
    // send it back to the senders
    (*test_func)(recvbuf, recvcount, MPI_DOUBLE,
                 backbuf, backcount, MPI_DOUBLE, MPI_COMM_WORLD);

    // now must be valid *sendbuf == *backbuf
    CU_TEST(memcmp(sendbuf, backbuf, buf_len) == 0);

    free(sendbuf);
    free(recvbuf);
    free(backbuf);
}


DECL_TESTFUNC(cr_alltoallv)
{
    L4C(__log4c_category_trace(GlobLogCat, "test_cr_alltoallv() -- begin"));

    test_alltoallv_impl(Cr_Alltoallv);

    L4C(__log4c_category_trace(GlobLogCat, "test_cr_alltoallv() -- end"));
}

DECL_TESTFUNC(cr_alltoallv_in_place)
{
    L4C(__log4c_category_trace(GlobLogCat,
                               "test_cr_alltoallv_in_place() -- begin"));

    test_alltoallv_in_place_impl(Cr_Alltoallv);

    L4C(__log4c_category_trace(GlobLogCat,
                               "test_cr_alltoallv_in_place() -- end"));

}


DECL_TESTFUNC(cr_alltoallv2)
{
    L4C(__log4c_category_trace(GlobLogCat, "test_cr_alltoallv2() -- begin"));

    test_alltoallv_impl(Cr_Alltoallv2);

    L4C(__log4c_category_trace(GlobLogCat, "test_cr_alltoallv2() -- end"));
}

DECL_TESTFUNC(cr_alltoallv2_in_place)
{
    L4C(__log4c_category_trace(GlobLogCat,
                               "test_cr_alltoallv2_in_place() -- begin"));

    test_alltoallv_in_place_impl(Cr_Alltoallv2);

    L4C(__log4c_category_trace(GlobLogCat,
                               "test_cr_alltoallv2_in_place() -- end"));
}


DECL_TESTFUNC(cr_alltoall)
{
#if 0
    L4C(__log4c_category_trace(GlobLogCat, "test_cr_alltoall() -- begin"));

    test_alltoall_impl(Cr_Alltoall);

    L4C(__log4c_category_trace(GlobLogCat, "test_cr_alltoall() -- end"));

#endif
}

DECL_TESTFUNC(mpi_alltoallv)
{
    L4C(__log4c_category_trace(GlobLogCat, "test_mpi_alltoallv() -- begin"));

    test_alltoallv_impl(MPI_Alltoallv);

    L4C(__log4c_category_trace(GlobLogCat, "test_mpi_alltoallv() -- end"));
}


DECL_TESTFUNC(mpi_alltoallv_in_place)
{
    L4C(__log4c_category_trace(GlobLogCat,
                               "test_mpi_alltoallv_in_place() -- begin"));

    test_alltoallv_in_place_impl(MPI_Alltoallv);

    L4C(__log4c_category_trace(GlobLogCat,
                               "test_mpi_alltoallv_in_place() -- end"));
}


DECL_TESTFUNC(mpi_alltoall)
{
    L4C(__log4c_category_trace(GlobLogCat, "test_mpi_alltoall() -- begin"));

    test_alltoall_impl(MPI_Alltoall);

    L4C(__log4c_category_trace(GlobLogCat, "test_mpi_alltoall() -- end"));

}
