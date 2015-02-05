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

#ifndef _CUNIT_MPI_INCLUDED_
#define _CUNIT_MPI_INCLUDED_

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

/* -------------------------------------------------------------------------- */

#define DECL_TESTFUNC(name) void test_ ## name (void)
#define CU_TESTINFO(name) { "test_" #name, test_ ## name }

int cunit_main( int argc, char *argv[],  CU_SuiteInfo *suites);

CU_ErrorCode CU_mpi_run_tests( void );

void CU_mpi_set_mode(CU_BasicRunMode mode);
CU_BasicRunMode CU_mpi_get_mode(void);

/* -------------------------------------------------------------------------- */

#endif // _CUNIT_MPI_INCLUDED_
