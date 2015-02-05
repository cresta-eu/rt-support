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

#ifndef MOD_CUBLAS_H_INCLUDED
#define MOD_CUBLAS_H_INCLUDED

#include "ipm_modules.h"
#include "cuda.h"
#include "cublas.h"

int mod_cublas_init(ipm_mod_t* mod, int flags);
int mod_cublas_output(ipm_mod_t* mod, int flags);

#define IPM_CUFFT_KEY(key_,call_,rank_,size_,reg_,csite_)     \
  KEY_CLEAR(key_);					      \
  KEY_SET_ACTIVITY(key_,call_);				      \
  KEY_SET_REGION(key_,reg_);				      \
  KEY_SET_RANK(key_,rank_);				      \
  KEY_SET_BYTES(key_,size_);				      \
  KEY_SET_CALLSITE(key_,csite_);

#define IPM_CUBLAS_BYTES_NONE_C(bytes_)        bytes_=0;
#define IPM_CUBLAS_BYTES_MNK_C(bytes_)         bytes_=m*n*k;
#define IPM_CUBLAS_BYTES_NELEMSIZE_C(bytes_)   bytes_=n*elemSize;


typedef struct cublasdata
{
  double time;
  double time_e;
} cublasdata_t;

#endif /* MOD_CUFFT_H_INCLUDED */

