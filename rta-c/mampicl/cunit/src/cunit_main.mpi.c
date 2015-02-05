/*
 * mampicl - message passing communication routines library.
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

#include <stdio.h>

#include <mpi.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "cunit_mpi.h"
#include "logging.h"

int cunit_main( int argc, char *argv[],  CU_SuiteInfo *suites)
{
    int myrank, numprocs;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    log_init(myrank);

    CU_initialize_registry();
    CU_register_suites(suites);

    CU_mpi_set_mode(CU_BRM_VERBOSE);
    CU_mpi_run_tests();

    CU_cleanup_registry();

    log_fini();
    MPI_Finalize();

    return 0;
}
