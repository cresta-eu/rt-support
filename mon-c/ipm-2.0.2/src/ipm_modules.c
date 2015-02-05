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

#include "ipm_core.h"
#include "ipm_sizes.h"
#include "ipm_modules.h"


ipm_mod_t modules[MAXNUM_MODULES];

void ipm_module_init(struct ipm_module *mod)
{   
  mod->state=STATE_NOTINIT;
  mod->init=0;
  mod->output=0;
  mod->finalize=0;
  mod->xml=0;
  mod->regfunc=0;
  mod->name=0;
  mod->ct_offs=0;
  mod->ct_range=0;
}
