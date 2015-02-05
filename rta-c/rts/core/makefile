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

-include ../make_init.mk


ARTEFACT = $(MK_CONFIG)/rts.a

# Define default target before including more definitions.
all : $(ARTEFACT)

project : all

-include ../make_proj.mk

# Artefacts
#
# Build a library
$(ARTEFACT) : $(OBJS)
	ar -ruc $(ARTEFACT) $(OBJS)

# Build an executable
#$(ARTEFACT) : $(REF_LIBS) $(OBJS)
#        $(MPICXX) $(LDFLAGS) $(LDFLAGS_OS) $(LDFLAGS_USER) -o "$(ARTEFACT)" $(SPEC_OBJS) $(OBJS) $(REF_LIBS) $(EXT_LIBS)


# Clean targets
.PHONY : projclean # refclean

clean : projclean # refclean
#
#refclean :
#	( cd ../app ; make clean )
#
projclean :
	rm -f $(C_OBJS) $(CPP_OBJS) $(C_DEPS) $(CPP_DEPS) $(ARTEFACT)


# Referenced projects
#
#../app/$(MK_CONFIG)/mdbase.a ::
#	( cd ../app ; make $(MK_CONFIG)/mdbase.a )
