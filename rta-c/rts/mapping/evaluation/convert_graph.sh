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

if [ $# -ne 2 ]
	then
		echo "usage: ./convert_graph <num vertices> <input filename>"
else
	NUM_VERTICES=$1
	FILE_NAME=$2
	echo "$1" && wc -l "$2" | awk '{print $1}' && cat "$2" | awk '{print $2, $4, substr($8,0,5) * 1000}'
fi

