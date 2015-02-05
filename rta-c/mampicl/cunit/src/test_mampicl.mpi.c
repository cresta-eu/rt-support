

#include <stdio.h>

#include <mpi.h>

#include "cunit_mpi.h"

/* Define test suites */
#define SUITE_DEFINITION(name)          \
extern CU_TestInfo tests_ ## name [];    \
extern int init_suite_ ## name (void);   \
extern int cleanup_suite_ ## name (void);

SUITE_DEFINITION(mpi_coll)
SUITE_DEFINITION(mpi_topo)
SUITE_DEFINITION(mpi_neighbor_coll)


/* Prepare test collection for runner */
CU_SuiteInfo mpi_suites[] = {
    { "MPI Collectives",
      init_suite_mpi_coll, cleanup_suite_mpi_coll, tests_mpi_coll
    },
    { "MPI Topologies",
      init_suite_mpi_topo, cleanup_suite_mpi_topo,
      tests_mpi_topo
    },
    { "MPI Neighbor Collectives",
      init_suite_mpi_neighbor_coll, cleanup_suite_mpi_neighbor_coll,
      tests_mpi_neighbor_coll
    },
    CU_SUITE_INFO_NULL
};


int main( int argc, char *argv[] )
{
    return cunit_main(argc, argv, mpi_suites);
}
