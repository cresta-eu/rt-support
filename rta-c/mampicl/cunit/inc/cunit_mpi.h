
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
