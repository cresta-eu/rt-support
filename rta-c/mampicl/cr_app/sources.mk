# Yet another option for custimisation.
#
CFLAGS += -I ../crystal/src
# CPPFLAGS += ...

# Specifikation of C modules.
#
C_MODULES += \
main.mpi

# Specification of C++ modules.
#
# CPP_MODULES += \
# <module-name> \

# Specifikation of module-specific options.
#
# <module-name>_CFLAGS = ...
# <module-name>_CPPFLAGS = ...

# Referenced projects that must be built before the own built.
#
REF_PROJECTS += \
../crystal \
../commprim

# Additional libraries for linking
#
# EXT_LIBS += \
# -lm \
