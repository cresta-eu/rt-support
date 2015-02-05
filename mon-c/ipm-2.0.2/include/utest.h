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

#ifndef UTEST_H_INCLUDED
#define UTEST_H_INCLUDED

#define VERIFY(myrank_, call_, bytes_, orank_, region_, count_)		\
  fprintf(stdout, "%03d.VERIFY: call=\"%s\" bytes=\"%d\" orank=\"%d\" region=\"%d\" count=\"%d\"\n", \
	  myrank_, call_, bytes_, orank_, region_, count_);


#endif /* UTEST_H_INCLUDED */
