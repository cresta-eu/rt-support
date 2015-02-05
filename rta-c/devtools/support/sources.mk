#
# rta-c - runtime system administration component.
#  Copyright (C) 2014  Michael Schliephake
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

#CUNIT_HOME = $(MICHS_USR)/$(CC_NAME)/$(CC_VERSION)


C_MODULES += \
bits \
cunit_main.mpi \
cunit_mode.mpi \
time_measurement

cunit_mode_CFLAGS = -I$(DEVLIBS)/include/CUnit
