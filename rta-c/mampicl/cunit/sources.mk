#
# mampicl - message passing communication routines library.
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

# Yet another option for custimisation.
#
# CFLAGS +=
# CPPFLAGS +=

# Specifikation of C modules.
#
C_MODULES += \
cunit_main.mpi \
cunit_mode.mpi \
test_mampicl.mpi \
suite_a2a.mpi \
suite_mpincoll.mpi \
suite_mpitopo.mpi

# Specification of C++ modules.
#
# CPP_MODULES += \
# <module-name> \

# Specifikation of module-specific options.
#
# <module-name>_CFLAGS = ...
# <module-name>_CPPFLAGS = ...
cunit_mode_CFLAGS = -I$(DEVLIBS)/include/CUnit

# Referenced projects that must be built before the own built.
#
REF_PROJECTS += \
../crystal \
../commprim

# Additional libraries for linking
#
EXT_LIBS += \
-lcunit
