

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

#include "cunit_mpi.h"

#include "cr_mpi.h"
#include "logging.h"

/* ------------------------------------------------------------------------- */

typedef int (*Alltoallv_func)(
    const void *sendbuf, const int *sendcounts, const int *sdispls,
        MPI_Datatype sendtype,
    void *recvbuf, const int *recvcounts, const int *rdispls,
        MPI_Datatype recvtype,
    MPI_Comm comm);

typedef int (*Alltoall_func)(
    const void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype,
    MPI_Comm comm);

/* ------------------------------------------------------------------------- */

/*
 * Suite name : Alltoall_MPI
 */
/* ------------------------------------------------------------------------- */

static int numprocs, myrank;

// Forward declarations of tests.
DECL_TESTFUNC(mpi_neighbor_collectives);
DECL_TESTFUNC(michs_neighbor_alltoallv);


// Definition of the test suite
CU_TestInfo tests_mpi_neighbor_coll[] = {
    CU_TESTINFO(mpi_neighbor_collectives),
    CU_TESTINFO(michs_neighbor_alltoallv),
    CU_TEST_INFO_NULL
};

/* ------------------------------------------------------------------------- */
/* Suite management */

int init_suite_mpi_neighbor_coll( void )
{
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Cr_Init(MPI_COMM_WORLD);

    return 0;
}

int cleanup_suite_mpi_neighbor_coll( void )
{
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Test implementations */

#if !defined(USE_STRICT_MPI) && defined(MPICH)
#define TEST_NEIGHB_COLL 1
#endif

/* assert-like macro that bumps the err count and emits a message */
#define CHECK(x_)                                                    \
    do {                                                             \
        if (!(x_)) {                                                 \
            ++errs;                                                  \
            if (errs < 10) {                                         \
                sprintf(errmsg,                                      \
                    "check failed: (%s), line %d", #x_, __LINE__);   \
                CU_FAIL(errmsg);                                     \
            }                                                        \
        }                                                            \
    } while (0)

DECL_TESTFUNC(mpi_neighbor_collectives)
{
#if defined(TEST_NEIGHB_COLL)
    int errs = 0;
    int wrank, wsize;
    int periods[1] = { 0 };
    MPI_Comm cart /*, dgraph, graph*/;
    char errmsg[1024];

    MPI_Comm_rank(MPI_COMM_WORLD, &wrank);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    /* a basic test for the 10 (5 patterns x {blocking,nonblocking}) MPI-3
     * neighborhood collective routines */

    /* (wrap)--> 0 <--> 1 <--> ... <--> p-1 <--(wrap) */
    MPI_Cart_create(MPI_COMM_WORLD, 1, &wsize, periods, /*reorder=*/0, &cart);

    /* allgather */
    {
        int sendbuf[1] = { wrank };
        int recvbuf[2] = { 0xdeadbeef, 0xdeadbeef };

        /* should see one send to each neighbor (rank-1 and rank+1) and one receive
         * each from same */
        MPI_Neighbor_allgather(sendbuf, 1, MPI_INT, recvbuf, 1, MPI_INT, cart);

        if (wrank == 0)
            CHECK(recvbuf[0] == 0xdeadbeef);
        else
            CHECK(recvbuf[0] == wrank - 1);

        if (wrank == wsize - 1)
            CHECK(recvbuf[1] == 0xdeadbeef);
        else
            CHECK(recvbuf[1] == wrank + 1);
    }

    /* allgatherv */
    {
        int sendbuf[1]    = { wrank };
        int recvbuf[2]    = { 0xdeadbeef, 0xdeadbeef };
        int recvcounts[2] = { 1, 1 };
        int displs[2]     = { 1, 0};

        /* should see one send to each neighbor (rank-1 and rank+1) and one receive
         * each from same, but put them in opposite slots in the buffer */
        MPI_Neighbor_allgatherv(sendbuf, 1, MPI_INT, recvbuf, recvcounts, displs, MPI_INT, cart);

        if (wrank == 0)
            CHECK(recvbuf[1] == 0xdeadbeef);
        else
            CHECK(recvbuf[1] == wrank - 1);

        if (wrank == wsize - 1)
            CHECK(recvbuf[0] == 0xdeadbeef);
        else
            CHECK(recvbuf[0] == wrank + 1);
    }

    /* alltoall */
    {
        int sendbuf[2]    = { -(wrank+1), wrank+1 };
        int recvbuf[2]    = { 0xdeadbeef, 0xdeadbeef };

        /* should see one send to each neighbor (rank-1 and rank+1) and one
         * receive each from same */
        MPI_Neighbor_alltoall(sendbuf, 1, MPI_INT, recvbuf, 1, MPI_INT, cart);

        if (wrank == 0)
            CHECK(recvbuf[0] == 0xdeadbeef);
        else
            CHECK(recvbuf[0] == wrank);

        if (wrank == wsize - 1)
            CHECK(recvbuf[1] == 0xdeadbeef);
        else
            CHECK(recvbuf[1] == -(wrank + 2));
    }

    /* alltoallv */
    {
        int sendbuf[2]    = { -(wrank+1), wrank+1 };
        int recvbuf[2]    = { 0xdeadbeef, 0xdeadbeef };
        int sendcounts[2] = { 1, 1 };
        int recvcounts[2] = { 1, 1 };
        int sdispls[2]    = { 0, 1 };
        int rdispls[2]    = { 1, 0 };

        /* should see one send to each neighbor (rank-1 and rank+1) and one receive
         * each from same, but put them in opposite slots in the buffer */
        MPI_Neighbor_alltoallv(sendbuf, sendcounts, sdispls, MPI_INT,
                                recvbuf, recvcounts, rdispls, MPI_INT,
                                cart);

        if (wrank == 0)
            CHECK(recvbuf[1] == 0xdeadbeef);
        else
            CHECK(recvbuf[1] == wrank);

        if (wrank == wsize - 1)
            CHECK(recvbuf[0] == 0xdeadbeef);
        else
            CHECK(recvbuf[0] == -(wrank + 2));
    }

    /* alltoallw */
    {
        int sendbuf[2]            = { -(wrank+1), wrank+1 };
        int recvbuf[2]            = { 0xdeadbeef, 0xdeadbeef };
        int sendcounts[2]         = { 1, 1 };
        int recvcounts[2]         = { 1, 1 };
        MPI_Aint sdispls[2]       = { 0, sizeof(int) };
        MPI_Aint rdispls[2]       = { sizeof(int), 0 };
        MPI_Datatype sendtypes[2] = { MPI_INT, MPI_INT };
        MPI_Datatype recvtypes[2] = { MPI_INT, MPI_INT };

        /* should see one send to each neighbor (rank-1 and rank+1) and one receive
         * each from same, but put them in opposite slots in the buffer */
        MPI_Neighbor_alltoallw(sendbuf, sendcounts, sdispls, sendtypes,
                                recvbuf, recvcounts, rdispls, recvtypes,
                                cart);

        if (wrank == 0)
            CHECK(recvbuf[1] == 0xdeadbeef);
        else
            CHECK(recvbuf[1] == wrank);

        if (wrank == wsize - 1)
            CHECK(recvbuf[0] == 0xdeadbeef);
        else
            CHECK(recvbuf[0] == -(wrank + 2));
    }


    MPI_Comm_free(&cart);

    MPI_Reduce((wrank == 0 ? MPI_IN_PLACE : &errs), &errs, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (wrank == 0) {
        if (errs) {
            sprintf(errmsg, "found %d errors\n", errs);
            CU_FAIL(errmsg);
        }
    }
#else
    CU_PASS("MPI-3 not available.")
#endif /* defined(TEST_NEIGHB_COLL) */
}

static void
check_communicator(
    MPI_Comm comm, int deg_creation, int *srcs_creation, int *dests_creation)
{
    int ideg, odeg, weighted;

    MPI_Dist_graph_neighbors_count(comm, &ideg, &odeg, &weighted);
    CU_ASSERT_EQUAL(ideg, odeg);
    CU_ASSERT_EQUAL(odeg, deg_creation);
    CU_ASSERT_TRUE(weighted);

    int dests[odeg], srcs[ideg];

    MPI_Dist_graph_neighbors(comm, ideg, srcs, MPI_UNWEIGHTED,
                                   odeg, dests, MPI_UNWEIGHTED);
    CU_ASSERT(memcmp(srcs, srcs_creation, odeg*sizeof(int)) == 0);
    CU_ASSERT(memcmp(dests, dests_creation, odeg*sizeof(int)) == 0);
}

DECL_TESTFUNC(michs_neighbor_alltoallv)
{
#if defined(TEST_NEIGHB_COLL)
    MPI_Barrier(MPI_COMM_WORLD);
    /* Communicator defined via outgoing edges. */
    MPI_Comm graph_comm;

    int deg = ((numprocs - 1)>>1)<<1;
    int dests[deg], weights[deg], displs[deg], counts[deg];
    int sendbuf[deg], recvbuf[deg], recvbuf2[deg];

    for (int i = 0; i < deg/2; i++)
    {
        // Neighbour downwards
        dests[2*i]   = (myrank - (i+1) + numprocs)%numprocs;
        sendbuf[2*i] = dests[2*i];
        weights[2*i] = 1;
        displs[2*i]  = 2*i;
        counts[2*i]  = 1;
        // Neighbour upwards
        dests[2*i+1]   = (myrank + (i+1))%numprocs;
        sendbuf[2*i+1] = dests[2*i+1];
        weights[2*i+1] = 1;
        displs[2*i+1]  = 2*i+1;
        counts[2*i+1]  = 1;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD,
                                   deg, dests, weights, deg, dests, weights,
                                   MPI_INFO_NULL, 0, &graph_comm);

    MPI_Barrier(MPI_COMM_WORLD);

    check_communicator(graph_comm, deg, dests, dests);

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Neighbor_alltoallv(sendbuf, counts, displs, MPI_INT,
                           recvbuf, counts, displs, MPI_INT,
                           graph_comm);
    for (int i = 0; i < deg; i++)
        CU_ASSERT_EQUAL(recvbuf[i], myrank);

    MPI_Neighbor_alltoallv(recvbuf, counts, displs, MPI_INT,
                           recvbuf2, counts, displs, MPI_INT,
                           graph_comm);
    for (int i = 0; i < deg; i++)
        CU_ASSERT_EQUAL(sendbuf[i], recvbuf2[i]);

    MPI_Comm_free(&graph_comm);
#else /* defined(TEST_NEIGHB_COLL) */
    CU_PASS("MPI-3 not available.")
#endif /* defined(TEST_NEIGHB_COLL) */
}
