

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

#include "cunit_mpi.h"
#include "logging.h"

/* ------------------------------------------------------------------------- */
/*
 * Suite name : mpi_topo
 */
/* ------------------------------------------------------------------------- */

static int numprocs, myrank;

// Forward declarations of tests.
DECL_TESTFUNC(mpi_dist_graph_create);
DECL_TESTFUNC(mpi_dist_graph_unweighted);

// Definition of the test suite
CU_TestInfo tests_mpi_topo[] = {
    CU_TESTINFO(mpi_dist_graph_create),
    CU_TESTINFO(mpi_dist_graph_unweighted),
    CU_TEST_INFO_NULL
};

/* ------------------------------------------------------------------------- */
/* Suite management */

int init_suite_mpi_topo( void )
{
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Cr_Init(MPI_COMM_WORLD);

    return 0;
}

int cleanup_suite_mpi_topo( void )
{
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Test implementations */

#define NUM_GRAPHS 10
#define MAX_WEIGHT 100

/* convenience globals */
static int size, rank;
/* Maybe use a bit vector instead? */
static int **layout;
#define MAX_LAYOUT_NAME_LEN 256
static char graph_layout_name[MAX_LAYOUT_NAME_LEN] = {'\0'};

static void
create_graph_layout(int graph_num)
{
    int i, j;

    if (rank == 0) {
        switch (graph_num) {
            case 0:
                strncpy(graph_layout_name, "deterministic complete graph", MAX_LAYOUT_NAME_LEN);
                for (i = 0; i < size; i++)
                    for (j = 0; j < size; j++)
                        layout[i][j] = (i + 2) * (j + 1);
                break;
            case 1:
                strncpy(graph_layout_name, "every other edge deleted", MAX_LAYOUT_NAME_LEN);
                for (i = 0; i < size; i++)
                    for (j = 0; j < size; j++)
                        layout[i][j] = (j % 2 ? (i + 2) * (j + 1) : 0);
                break;
            case 2:
                strncpy(graph_layout_name, "only self-edges", MAX_LAYOUT_NAME_LEN);
                for (i = 0; i < size; i++) {
                    for (j = 0; j < size; j++) {
                        if (i == rank && j == rank)
                            layout[i][j] = 10 * (i + 1);
                        else
                            layout[i][j] = 0;
                    }
                }
                break;
            case 3:
                strncpy(graph_layout_name, "no edges", MAX_LAYOUT_NAME_LEN);
                for (i = 0; i < size; i++)
                    for (j = 0; j < size; j++)
                        layout[i][j] = 0;
                break;
            default:
                strncpy(graph_layout_name, "a random incomplete graph", MAX_LAYOUT_NAME_LEN);
                srand(graph_num);

                /* Create a connectivity graph; layout[i,j]==w represents an outward
                 * connectivity from i to j with weight w, w==0 is no edge. */
                for (i = 0; i < size; i++) {
                    for (j = 0; j < size; j++) {
                        /* disable about a third of the edges */
                        if (((rand() * 1.0) / RAND_MAX) < 0.33)
                            layout[i][j] = 0;
                        else
                            layout[i][j] = rand() % MAX_WEIGHT;
                    }
                }
                break;
        }
    }

    /* because of the randomization we must determine the graph on rank 0 and
     * send the layout to all other processes */
    MPI_Bcast(graph_layout_name, MAX_LAYOUT_NAME_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);
    for (i = 0; i < size; ++i) {
        MPI_Bcast(layout[i], size, MPI_INT, 0, MPI_COMM_WORLD);
    }
}

static int
verify_comm(MPI_Comm comm)
{
    int local_errs = 0;
    int i, j;
    int indegree, outdegree, weighted;
    int *sources, *sweights, *destinations, *dweights;
    int use_dup;
    int topo_type = MPI_UNDEFINED;
    MPI_Comm dupcomm = MPI_COMM_NULL;
    char errmsg[1024];

    sources = (int *) malloc(size * sizeof(int));
    sweights = (int *) malloc(size * sizeof(int));
    destinations = (int *) malloc(size * sizeof(int));
    dweights = (int *) malloc(size * sizeof(int));

    for (use_dup = 0; use_dup <= 1; ++use_dup) {
        if (!use_dup) {
            MPI_Dist_graph_neighbors_count(comm, &indegree, &outdegree, &weighted);
        }
        else {
            MPI_Comm_dup(comm, &dupcomm);
            comm = dupcomm; /* caller retains original comm value */
        }

        MPI_Topo_test(comm, &topo_type);
        if (topo_type != MPI_DIST_GRAPH) {
            CU_FAIL("topo_type != MPI_DIST_GRAPH");
            ++local_errs;
        }

        j = 0;
        for (i = 0; i < size; i++)
            if (layout[i][rank])
                j++;
        if (j != indegree) {
            sprintf(errmsg, "indegree does not match, expected=%d got=%d, layout='%s'", indegree, j, graph_layout_name);
            CU_FAIL(errmsg);
            ++local_errs;
        }

        j = 0;
        for (i = 0; i < size; i++)
            if (layout[rank][i])
                j++;
        if (j != outdegree) {
            sprintf(errmsg, "outdegree does not match, expected=%d got=%d, layout='%s'", outdegree, j, graph_layout_name);
            CU_FAIL(errmsg);
            ++local_errs;
        }

        if ((indegree || outdegree) && (weighted == 0)) {
            sprintf(errmsg, "MPI_Dist_graph_neighbors_count thinks the graph is not weighted");
            CU_FAIL(errmsg);
            ++local_errs;
        }


        MPI_Dist_graph_neighbors(comm, indegree, sources, sweights, outdegree, destinations, dweights);

        /* For each incoming and outgoing edge in the matrix, search if
         * the query function listed it in the sources. */
        for (i = 0; i < size; i++) {
            if (layout[i][rank]) {
                for (j = 0; j < indegree; j++) {
                    CU_ASSERT(sources[j] >= 0);
                    CU_ASSERT(sources[j] < size);
                    if (sources[j] == i)
                        break;
                }
                if (j == indegree) {
                    sprintf(errmsg, "no edge from %d to %d specified", i, rank);
                    CU_FAIL(errmsg);
                    ++local_errs;
                }
                else {
                    if (sweights[j] != layout[i][rank]) {
                        sprintf(errmsg, "incorrect weight for edge (%d,%d): %d instead of %d",
                                i, rank, sweights[j], layout[i][rank]);
                        CU_FAIL(errmsg);
                        ++local_errs;
                    }
                }
            }
            if (layout[rank][i]) {
                for (j = 0; j < outdegree; j++) {
                    CU_ASSERT(destinations[j] >= 0);
                    CU_ASSERT(destinations[j] < size);
                    if (destinations[j] == i)
                        break;
                }
                if (j == outdegree) {
                    sprintf(errmsg, "no edge from %d to %d specified", rank, i);
                    CU_FAIL(errmsg);
                    ++local_errs;
                }
                else {
                    if (dweights[j] != layout[rank][i]) {
                        sprintf(errmsg, "incorrect weight for edge (%d,%d): %d instead of %d",
                                rank, i, dweights[j], layout[rank][i]);
                        CU_FAIL(errmsg)
                        ++local_errs;
                    }
                }
            }
        }

        /* For each incoming and outgoing edge in the sources, we should
         * have an entry in the matrix */
        for (i = 0; i < indegree; i++) {
            if (layout[sources[i]][rank] != sweights[i]) {
                sprintf(errmsg, "edge (%d,%d) has a weight %d instead of %d", i, rank,
                        sweights[i], layout[sources[i]][rank]);
                CU_FAIL(errmsg)
                ++local_errs;
            }
        }
        for (i = 0; i < outdegree; i++) {
            if (layout[rank][destinations[i]] != dweights[i]) {
                sprintf(errmsg, "edge (%d,%d) has a weight %d instead of %d", rank, i,
                        dweights[i], layout[rank][destinations[i]]);
                CU_FAIL(errmsg);
                ++local_errs;
            }
        }

    }

    if (dupcomm != MPI_COMM_NULL)
        MPI_Comm_free(&dupcomm);

    return local_errs;
}

DECL_TESTFUNC(mpi_dist_graph_create)
{
    L4C(__log4c_category_trace(GlobLogCat,
                               "test_mpi_dist_graph_create() -- begin"));

    int errs = 0;
    int i, j, k, p;
    int indegree, outdegree, reorder;
    int check_indegree, check_outdegree, check_weighted;
    int *sources, *sweights, *destinations, *dweights, *degrees;
    MPI_Comm comm;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    layout = (int **) malloc(size * sizeof(int *));
    CU_ASSERT_PTR_NOT_NULL(layout);
    for (i = 0; i < size; i++) {
        layout[i] = (int *) malloc(size * sizeof(int));
        CU_ASSERT_PTR_NOT_NULL(layout[i]);
    }
    /* alloc size*size ints to handle the all-on-one-process case */
    sources = (int *) malloc(size * size * sizeof(int));
    sweights = (int *) malloc(size * size * sizeof(int));
    destinations = (int *) malloc(size * size * sizeof(int));
    dweights = (int *) malloc(size * size * sizeof(int));
    degrees = (int *) malloc(size * size * sizeof(int));

    for (i = 0; i < NUM_GRAPHS; i++) {
        create_graph_layout(i);
        if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "using graph layout '%s'", graph_layout_name));
        }

        /* MPI_Dist_graph_create_adjacent */
        if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create_adjacent"));
        }
        indegree = 0;
        k = 0;
        for (j = 0; j < size; j++) {
            if (layout[j][rank]) {
                indegree++;
                sources[k] = j;
                sweights[k++] = layout[j][rank];
            }
        }

        outdegree = 0;
        k = 0;
        for (j = 0; j < size; j++) {
            if (layout[rank][j]) {
                outdegree++;
                destinations[k] = j;
                dweights[k++] = layout[rank][j];
            }
        }

        for (reorder = 0; reorder <= 1; reorder++) {
            MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD,
                indegree, sources, sweights,
                outdegree, destinations, dweights, MPI_INFO_NULL,
                reorder, &comm);
            MPI_Barrier(comm);
            errs += verify_comm(comm);
            MPI_Comm_free(&comm);
        }

        /* a weak check that passing MPI_UNWEIGHTED doesn't cause
         * create_adjacent to explode */
        MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD,
            indegree, sources, MPI_UNWEIGHTED,
            outdegree, destinations, MPI_UNWEIGHTED, MPI_INFO_NULL,
            reorder, &comm);
        MPI_Barrier(comm);
        /* intentionally no verify here, weights won't match */
        MPI_Comm_free(&comm);

        /* MPI_Dist_graph_create() where each process specifies its
         * outgoing edges */
        if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ outgoing only" ));
        }
        sources[0] = rank;
        k = 0;
        for (j = 0; j < size; j++) {
            if (layout[rank][j]) {
                destinations[k] = j;
                dweights[k++] = layout[rank][j];
            }
        }
        degrees[0] = k;
        for (reorder = 0; reorder <= 1; reorder++) {
            MPI_Dist_graph_create(MPI_COMM_WORLD, 1, sources, degrees, destinations, dweights,
                                  MPI_INFO_NULL, reorder, &comm);
            MPI_Barrier(comm);
            errs += verify_comm(comm);
            MPI_Comm_free(&comm);
        }

        /* MPI_Dist_graph_create() where each process specifies its
         * incoming edges */
        if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ incoming only"));
        }
        k = 0;
        for (j = 0; j < size; j++) {
            if (layout[j][rank]) {
                sources[k] = j;
                sweights[k] = layout[j][rank];
                degrees[k] = 1;
                destinations[k++] = rank;
            }
        }
        for (reorder = 0; reorder <= 1; reorder++) {
            MPI_Dist_graph_create(MPI_COMM_WORLD, k, sources, degrees, destinations, sweights,
                                  MPI_INFO_NULL, reorder, &comm);
            MPI_Barrier(comm);
            errs += verify_comm(comm);
            MPI_Comm_free(&comm);
        }

        /* MPI_Dist_graph_create() where rank 0 specifies the entire
         * graph */
        if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ rank 0 specifies only"));
        }
        p = 0;
        for (j = 0; j < size; j++) {
            for (k = 0; k < size; k++) {
                if (layout[j][k]) {
                    sources[p] = j;
                    sweights[p] = layout[j][k];
                    degrees[p] = 1;
                    destinations[p++] = k;
                }
            }
        }
        for (reorder = 0; reorder <= 1; reorder++) {
            MPI_Dist_graph_create(MPI_COMM_WORLD, (rank == 0) ? p : 0, sources, degrees,
                                  destinations, sweights, MPI_INFO_NULL, reorder, &comm);
            MPI_Barrier(comm);
            errs += verify_comm(comm);
            MPI_Comm_free(&comm);
        }

        /* MPI_Dist_graph_create() where rank 0 specifies the entire
         * graph and all other ranks pass NULL.  Can catch implementation
         * problems when MPI_UNWEIGHTED==NULL. */
        if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ rank 0 specifies only "
                    "-- NULLs"));
        }
        p = 0;
        for (j = 0; j < size; j++) {
            for (k = 0; k < size; k++) {
                if (layout[j][k]) {
                    sources[p] = j;
                    sweights[p] = layout[j][k];
                    degrees[p] = 1;
                    destinations[p++] = k;
                }
            }
        }
        for (reorder = 0; reorder <= 1; reorder++) {
            if (rank == 0) {
                MPI_Dist_graph_create(MPI_COMM_WORLD, p, sources, degrees,
                                      destinations, sweights, MPI_INFO_NULL, reorder, &comm);
            }
            else {
                MPI_Dist_graph_create(MPI_COMM_WORLD, 0, NULL, NULL,
                                      NULL, NULL, MPI_INFO_NULL, reorder, &comm);
            }
            MPI_Barrier(comm);
            errs += verify_comm(comm);
            MPI_Comm_free(&comm);
        }

    }

    /* now tests that don't depend on the layout[][] array */

    /* The MPI-2.2 standard recommends implementations set
     * MPI_UNWEIGHTED==NULL, but this leads to an ambiguity.  The draft
     * MPI-3.0 standard specifically recommends _not_ setting it equal
     * to NULL. */
    if (MPI_UNWEIGHTED == NULL) {
        CU_FAIL("MPI_UNWEIGHTED should not be NULL");
        ++errs;
    }

    /* MPI_Dist_graph_create() with no graph */
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ no graph"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create(MPI_COMM_WORLD, 0, sources, degrees,
                              destinations, sweights, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        if (!check_weighted) {
            CU_FAIL("expected weighted == TRUE for the \"no graph\" case");
            ++errs;
        }
        MPI_Comm_free(&comm);
    }

    /* MPI_Dist_graph_create() with no graph -- passing MPI_WEIGHTS_EMPTY
       instead */
    /* NOTE that MPI_WEIGHTS_EMPTY was added in MPI-3 and does not
       appear before then.  This part of the test thus requires a check
       on the MPI major version */
#if MPI_VERSION >= 3
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ no graph"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create(MPI_COMM_WORLD, 0, sources, degrees,
                              destinations, MPI_WEIGHTS_EMPTY, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        if (!check_weighted) {
            CU_FAIL("expected weighted == TRUE for the \"no graph -- "
                    "MPI_WEIGHTS_EMPTY\" case");
            ++errs;
        }
        MPI_Comm_free(&comm);
    }
#endif

    /* MPI_Dist_graph_create() with no graph -- passing NULLs instead */
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ no graph -- NULLs"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create(MPI_COMM_WORLD, 0, NULL, NULL,
                              NULL, NULL, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        /* ambiguous if they are equal, only check when they are distinct values. */
        if (MPI_UNWEIGHTED != NULL) {
            if (!check_weighted) {
                CU_FAIL("expected weighted == TRUE for the \"no graph -- "
                        "NULLs\" case");
                ++errs;
            }
        }
        MPI_Comm_free(&comm);
    }

    /* MPI_Dist_graph_create() with no graph -- passing NULLs+MPI_UNWEIGHTED instead */
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create w/ no graph -- "
                    "NULLs+MPI_UNWEIGHTED"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create(MPI_COMM_WORLD, 0, NULL, NULL,
                              NULL, MPI_UNWEIGHTED, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        /* ambiguous if they are equal, only check when they are distinct values. */
        if (MPI_UNWEIGHTED != NULL) {
            if (check_weighted) {
                CU_FAIL("expected weighted == FALSE for the \"no graph -- "
                        "NULLs+MPI_UNWEIGHTED\" case");
                ++errs;
            }
        }
        MPI_Comm_free(&comm);
    }

    /* MPI_Dist_graph_create_adjacent() with no graph */
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                     "testing MPI_Dist_graph_create_adjacent w/ no graph"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD, 0, sources, sweights,
                              0, destinations, dweights, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        if (!check_weighted) {
            CU_FAIL("expected weighted == TRUE for the \"no graph\" case");
            ++errs;
        }
        MPI_Comm_free(&comm);
    }

    /* MPI_Dist_graph_create_adjacent() with no graph -- passing MPI_WEIGHTS_EMPTY instead */
    /* NOTE that MPI_WEIGHTS_EMPTY was added in MPI-3 and does not
       appear before then.  This part of the test thus requires a check
       on the MPI major version */
#if MPI_VERSION >= 3
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create_adjacent w/ no graph -- "
                    "MPI_WEIGHTS_EMPTY"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD, 0, sources, MPI_WEIGHTS_EMPTY,
                              0, destinations, MPI_WEIGHTS_EMPTY, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        if (!check_weighted) {
            CU_FAIL("expected weighted == TRUE for the \"no graph -- "
                    "MPI_WEIGHTS_EMPTY\" case");
            ++errs;
        }
        MPI_Comm_free(&comm);
    }
#endif

    /* MPI_Dist_graph_create_adjacent() with no graph -- passing NULLs instead */
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create_adjacent w/ no graph -- "
                    "NULLs"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD, 0, NULL, NULL,
                              0, NULL, NULL, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        /* ambiguous if they are equal, only check when they are distinct values. */
        if (MPI_UNWEIGHTED != NULL) {
            if (!check_weighted) {
                CU_FAIL("expected weighted == TRUE for the \"no graph -- "
                        "NULLs\" case");
                ++errs;
            }
        }
        MPI_Comm_free(&comm);
    }

    /* MPI_Dist_graph_create_adjacent() with no graph -- passing NULLs+MPI_UNWEIGHTED instead */
    if (rank == 0) {
            L4C(__log4c_category_info(GlobLogCat,
                    "testing MPI_Dist_graph_create_adjacent w/ no graph -- "
                    "NULLs+MPI_UNWEIGHTED"));
    }
    for (reorder = 0; reorder <= 1; reorder++) {
        MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD, 0, NULL, MPI_UNWEIGHTED,
                              0, NULL, MPI_UNWEIGHTED, MPI_INFO_NULL, reorder, &comm);
        MPI_Dist_graph_neighbors_count(comm, &check_indegree, &check_outdegree, &check_weighted);
        /* ambiguous if they are equal, only check when they are distinct values. */
        if (MPI_UNWEIGHTED != NULL) {
            if (check_weighted) {
                CU_FAIL("expected weighted == FALSE for the \"no graph -- "
                        "NULLs+MPI_UNWEIGHTED\" case");
                ++errs;
            }
        }
        MPI_Comm_free(&comm);
    }


    for (i = 0; i < size; i++)
        free(layout[i]);
    free(layout);

    L4C(__log4c_category_trace(GlobLogCat,
                               "test_mpi_dist_graph_create() -- end"));
}


/* ------------------------------------------------------------------------- */
#define RING_NUM_NEIGHBORS   2

static int
validate_dgraph(MPI_Comm dgraph_comm)
{
    int         comm_topo;
    int         src_sz, dest_sz;
    int         wgt_flag, ierr;
    int         srcs[RING_NUM_NEIGHBORS], dests[RING_NUM_NEIGHBORS];
    int        *src_wgts, *dest_wgts;

    int         world_rank, world_size;
    int         idx, nbr_sep;
    char        errmsg[1024];

    comm_topo = MPI_UNDEFINED;
    MPI_Topo_test(dgraph_comm, &comm_topo);
    switch (comm_topo) {
        case MPI_DIST_GRAPH :
            break;
        default:
            CU_FAIL("dgraph_comm is NOT of type MPI_DIST_GRAPH");
            return 0;
    }

    ierr = MPI_Dist_graph_neighbors_count(dgraph_comm,
                                          &src_sz, &dest_sz, &wgt_flag);
    if (ierr != MPI_SUCCESS) {
        CU_FAIL("MPI_Dist_graph_neighbors_count() fails!");
        return 0;
    }

    if (wgt_flag) {
        CU_FAIL("dgraph_comm is NOT created with MPI_UNWEIGHTED");
        return 0;
    }

    if (src_sz != RING_NUM_NEIGHBORS || dest_sz != RING_NUM_NEIGHBORS) {
        sprintf(errmsg, "source or destination edge array is not of size %d."
                        "src_sz = %d, dest_sz = %d",
                         RING_NUM_NEIGHBORS, src_sz, dest_sz);
        CU_FAIL(errmsg)
        return 0;
    }

    /*
       src_wgts and dest_wgts could be anything, e.g. NULL, since
       MPI_Dist_graph_neighbors_count() returns MPI_UNWEIGHTED.
       Since this program has a Fortran77 version, and standard Fortran77
       has no pointer and NULL, so use MPI_UNWEIGHTED for the weighted arrays.
    */
    src_wgts  = MPI_UNWEIGHTED;
    dest_wgts = MPI_UNWEIGHTED;
    ierr = MPI_Dist_graph_neighbors(dgraph_comm,
                                    src_sz, srcs, src_wgts,
                                    dest_sz, dests, dest_wgts);
    if (ierr != MPI_SUCCESS) {
        CU_FAIL("MPI_Dist_graph_neighbors() fails!");
        return 0;
    }

    /*
       Check if the neighbors returned from MPI are really
       the nearest neighbors within a ring.
    */
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    for (idx=0; idx < src_sz; idx++) {
        nbr_sep = abs(srcs[idx] - world_rank);
        if ( nbr_sep != 1 && nbr_sep != (world_size-1) ) {
            sprintf(errmsg, "srcs[%d]=%d is NOT a neighbor of my rank %d.",
                            idx, srcs[idx], world_rank);
            CU_FAIL(errmsg)
            return 0;
        }
    }
    for (idx=0; idx < dest_sz; idx++) {
        nbr_sep = abs(dests[idx] - world_rank);
        if ( nbr_sep != 1 && nbr_sep != (world_size-1) ) {
            sprintf(errmsg, "dests[%d]=%d is NOT a neighbor of my rank %d.",
                            idx, dests[idx], world_rank);
            CU_FAIL(errmsg);
            return 0;
        }
    }
    return 1;
}

/**
 * Specify a distributed graph of a bidirectional ring of the MPI_COMM_WORLD,
 * i.e. everyone only talks to left and right neighbors.
 */
DECL_TESTFUNC(mpi_dist_graph_unweighted)
{
    MPI_Comm    dgraph_comm;
    int         world_size, world_rank, ierr;

    int         src_sz, dest_sz;
    int         degs[1];
    int         srcs[RING_NUM_NEIGHBORS], dests[RING_NUM_NEIGHBORS];

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    degs[0]  = 2;
    srcs[0]  = world_rank;
    dests[0] = world_rank-1 <  0          ? world_size-1 : world_rank-1 ;
    dests[1] = world_rank+1 >= world_size ?            0 : world_rank+1 ;
    ierr = MPI_Dist_graph_create(MPI_COMM_WORLD, 1, srcs, degs, dests,
                                 MPI_UNWEIGHTED, MPI_INFO_NULL, 1,
                                 &dgraph_comm);
    if ( ierr != MPI_SUCCESS )  {
        CU_FAIL("MPI_Dist_graph_create() fails!");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (!validate_dgraph(dgraph_comm)) {
        CU_FAIL("MPI_Dist_graph_create() does NOT create "
                "a bidirectional ring graph!");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_Comm_free(&dgraph_comm);

    src_sz   = 2;
    srcs[0]  = world_rank-1 <  0          ? world_size-1 : world_rank-1 ;
    srcs[1]  = world_rank+1 >= world_size ?            0 : world_rank+1 ;
    dest_sz  = 2;
    dests[0] = world_rank-1 <  0          ? world_size-1 : world_rank-1 ;
    dests[1] = world_rank+1 >= world_size ?            0 : world_rank+1 ;
    ierr = MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD,
                                          src_sz, srcs, MPI_UNWEIGHTED,
                                          dest_sz, dests, MPI_UNWEIGHTED,
                                          MPI_INFO_NULL, 1, &dgraph_comm);
    if ( ierr != MPI_SUCCESS ) {
        CU_FAIL("MPI_Dist_graph_create_adjacent() fails!");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }
    if (!validate_dgraph(dgraph_comm)) {
        CU_FAIL("MPI_Dist_graph_create_adjacent() does NOT create "
                "a bidirectional ring graph!");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }
    MPI_Comm_free(&dgraph_comm);
}
