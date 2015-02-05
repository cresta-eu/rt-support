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

include ../make_init.mk


ARTEFACT = $(MK_CONFIG)/libdevtools.a

all : $(ARTEFACT)


-include ../make_proj.mk

INSTALL_DIR ?= $(MICHS_USR)

# Targets
$(ARTEFACT) : $(OBJS)
	ar -ruc $(ARTEFACT) $(OBJS)

install :
	mkdir -p $(INSTALL_DIR)/lib $(INSTALL_DIR)/include
	cp $(ARTEFACT) $(INSTALL_DIR)/lib/
	cp inc/* $(INSTALL_DIR)/include

clean :
	rm -f $(C_OBJS) $(C_DEPS) $(ARTEFACT)
