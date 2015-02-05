#
# Main makefile of the project
#
#


-include ../make_init.mk


ARTEFACT = $(MK_CONFIG)/cr_app

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
	$(MPICXX) $(LDFLAGS) $(LDFLAGS_OS) $(LDFLAGS_USER) $(MPILDFLAGS_USER) \
         -o "$(ARTEFACT)" $(SPEC_OBJS) $(OBJS) $(LIBES)


.PHONY : projclean refclean
#
clean : projclean refclean
#
refclean :
	( cd ../commprim ; make clean )
	( cd ../crystal ; make clean )
#
projclean :
	rm -f $(C_OBJS) $(CPP_OBJS) $(C_DEPS) $(ARTEFACT)


# Example of a referenced project
#
../commprim/$(MK_CONFIG)/libcommprim.a ::
	( cd ../commprim ; make $(MK_CONFIG)/libcommprim.a )

../crystal/$(MK_CONFIG)/libcrystal.a ::
	( cd ../crystal ; make $(MK_CONFIG)/libcrystal.a )

