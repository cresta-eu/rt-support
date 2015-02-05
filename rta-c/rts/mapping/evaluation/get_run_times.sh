#!/bin/bash
#
# rta-c - message passing communication routines library.
#  Copyright (C) 2014  Sebastian Tunstig
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

inputfile="processmapping.oXXXXXX"
for i in {0..49}
do
	cat ${inputfile} |grep -e 'mapping time*\|refinement time*' | sed -n "$(($((${i}*20))+1)) , $(($((${i}+1))*20))p" | awk '{print $3}' | awk '{s+=$1} END {print s}'
done;
