/******************************************************************\
* This library is free software: you can redistribute it and/or   *
* modify it under the terms of the GNU LGPL as published by       *       
* the Free Software Foundation, either version 3 of the License,  *
* or (at your option) any later version.                          *
*								  *	
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   *
* GNU Lesser General Public License for more details.		  *
*								  *
* You should have received a copy of the GNU Lesser General       *
*  Public License along with this program.  If not,               *
* see <http://www.gnu.org/licenses/>.                             *
\******************************************************************/

#include "ipm_sizes.h"
#include "calltable.h"


ipm_call_t ipm_calltable[MAXSIZE_CALLTABLE];

void init_calltable()
{
  int i;
  for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    ipm_calltable[i].name=0;
    ipm_calltable[i].attr=0;
  }
}
