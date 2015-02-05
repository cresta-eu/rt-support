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

-include ../$(PROJ_MAKE_CUSTOMIZATION)

#
# CFLAGS_USER, MPICFLAGS_USER
# CPPFLAGS_USER, MPICPPFLAGS_USER
# LDFLAGS_USER, MPILDFLAGS_USER
#

MPICFLAGS_USER   += -DMPI_ALLOWED=1
MPICPPFLAGS_USER += -DMPI_ALLOWED=1
MPILDFLAGS_USER  +=

# Pedantic handling of warnings
# CFLAGS_USER = -Wall -Wextra -Werror -pedantic

ifneq (, $(findstring $(CLUSTER),"lindgren milner"))
  ifeq ("$(CLUSTER)","lindgren")
    CFLAGS_USER += -mtune=barcelona
  else
    CFLAGS_USER += -march=corei7-avx -mtune=corei7-avx -DMPI3_ALLOWED=1
  endif

  ifdef USE_CRAYPAT
    CFLAGS_USER += -finstrument-functions
  endif
else
  CFLAGS_USER += -DMPI3_ALLOWED=1
endif


ifdef DEBUG

CFLAGS_USER   += -DUSE_LOG4C=1
CPPFLAGS_USER += -DUSE_LOG4C=1

EXT_LIBS += -llog4c

ifneq (,$(findstring $(CLUSTER),"lindgren milner"))
  EXT_LIBS += -lexpat
endif

endif

