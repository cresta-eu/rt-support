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

#
# Main makefile of the project
#
#


-include ../make_init.mk


ARTEFACT = $(MK_CONFIG)/test_mampicl

# Define default target before including more definitions.
all : $(ARTEFACT)


-include ../make_proj.mk

INSTALL_DIR ?= $(HOME)/devel/usr

# Examples for artefacts
#
# Build a library
#$(ARTEFACT) : $(OBJS)
#	ar -ruc $(ARTEFACT) $(OBJS)

# Build an executable
$(ARTEFACT) : $(REF_LIBS) $(OBJS)
	$(MPICC) $(LDFLAGS) $(LDFLAGS_OS) $(LDFLAGS_USER) $(MPILDFLAGS_USER) \
         -o "$(ARTEFACT)" $(SPEC_OBJS) $(OBJS) $(LIBES)

# Example of clean target
#
.PHONY : projclean refclean
#
clean : projclean refclean
#
refclean :
	( cd ../commprim ; make clean )
	( cd ../crystal ; make clean )

projclean :
	rm -f $(C_OBJS) $(CPP_OBJS) $(C_DEPS) $(ARTEFACT)


# Example of a referenced project
#
#../app/$(MK_CONFIG)/mdbase.a ::
#	( cd ../app ; make $(MK_CONFIG)/mdbase.a )

../commprim/$(MK_CONFIG)/libcommprim.a ::
	( cd ../commprim ; make $(MK_CONFIG)/libcommprim.a )

../crystal/$(MK_CONFIG)/libcrystal.a ::
	( cd ../crystal ; make $(MK_CONFIG)/libcrystal.a )
