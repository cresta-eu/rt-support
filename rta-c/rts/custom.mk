#
# rta-c - message passing communication routines library.
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

-include ../$(PROJ_MAKE_CUSTOMIZATION)


# Selection of the method how to bind the RTS to application
#
# Selection can be made separately for MPI functions and other system functions
#
# Values for MPI functions
#   MPIRTS_LINKWRAP - Use ld to wrap the functions
#                     (Subvalue: RTS_USE_PMPI - also for OS "Darwin")
#   MPIRTS_COMPILERWRAP - Wrap function calls from the self-compiled modules
#
# Values for other system functions
#   RTS_LINKWRAP - Use ld to wrap the functions
#   RTS_COMPILERWRAP - Wrap function calls from the self-compiled modules
#

ifeq ($(OS),Linux)
    RTS_BINDING ?= RTS_LINKWRAP
else
    RTS_BINDING ?= RTS_COMPILERWRAP
endif
RTSMPI_BINDING ?= MPI$(RTS_BINDING)

# RTSMPI_BINDING ?= PMPI_INTERFACE

CFLAGS_USER += -D$(RTS_BINDING)=1 -D$(RTSMPI_BINDING)=1
CPPFLAGS_USER += -D$(RTS_BINDING)=1 -D$(RTSMPI_BINDING)=1


# CFLAGS_USER += -fopenmp
# CFLAGS_USER += -DMPE_TRACE=1

# LDFLAGS_USER += -fopenmp
# LDFLAGS_USER += -mpe=mpilog


# ParaVer
#   -DPARAVER_TRACE=1
