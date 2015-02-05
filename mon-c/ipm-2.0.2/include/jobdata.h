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


void ipm_get_job_id(char *id, int len);

void ipm_get_job_user(char *user, int len);

void ipm_get_job_allocation(char *allocation, int len);

void ipm_get_mach_info(char *machi, int len);

void ipm_get_mach_name(char *machn, int len);

void ipm_get_exec_cmdline(char *cmdl, char *rpath);

void ipm_get_exec_md5sum(char *md5sum, char *rpath);
