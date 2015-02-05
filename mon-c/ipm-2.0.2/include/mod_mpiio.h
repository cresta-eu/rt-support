/******************************************************************\
* This library is free software: you can redistribute it and/or   *
* modify it under the terms of the GNU LGPL as published by       *       
* the Free Software Foundation, either version 3 of the License,  *
* or (at your option) any later version.                          *
*                                                                 *     
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   *
* GNU Lesser General Public License for more details.             *
*                                                                 *
* You should have received a copy of the GNU Lesser General       *
*  Public License along with this program.  If not,               *
* see <http://www.gnu.org/licenses/>.                             *
\******************************************************************/

#ifndef MPIIO_H_INCLUDED
#define MPIIO_H_INCLUDED

#include <mpi.h>
#include "ipm_modules.h"

int mod_mpiio_init(ipm_mod_t* mod, int flags);

typedef struct mpiiodata
{
  double iotime;
  double iotime_e;
} mpiiodata_t;

#define IPM_MPIIO_RANK_NONE_C(rank_)    rank_=IPM_MPI_RANK_NORANK;
#define IPM_MPIIO_RANK_NONE_F(rank_)    rank_=IPM_MPI_RANK_NORANK;

extern mpiiodata_t mpiiodata[MAXNUM_REGIONS];

#define IPM_MPIIO_BYTES_NONE_C( bytes_ ) \
  bytes_=0;

#define IPM_MPIIO_BYTES_COUNT_DATATYPE_C( bytes_ ) \
  {						   \
    PMPI_Type_size(datatype, &bytes_);		   \
    bytes_ *= count;				   \
  }
   
#endif /* MPIIO_H_INCLUDED */
