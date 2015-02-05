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

# Include a personal makefile for customization. Don't add it to the official
# repository! Therefore add the name to your .hgignore file or create it
# outside of the source tree and define the path to it at the command line or
# in the environment with the variable PROJ_MAKE_CUSTOMIZATION.

PROJ_MAKE_CUSTOMIZATION ?= ../custom.mk
-include $(PROJ_MAKE_CUSTOMIZATION)
#
# Possible variable definitions to customize the build:
#
# CFLAGS_USER   Options to modify the compilation
# CPPFLAGS_USER
# LDFLAGS_USER  Options to modify the link step
# USER_OBJS	List of object files to link with the application.

PRJ_LOC=src
-include sources.mk

### End of reading external files ###

MPICC ?= mpicc
MPICXX ?= mpicxx
CC ?= gcc
CXX ?= g++

ifdef DEBUG
MK_CONFIG ?= debug
endif
MK_CONFIG ?= release

#Compiler options
## General
CFLAGS += -c -Wall -std=c99 -fmessage-length=0
## Dependency files
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"

ifdef DEBUG
CFLAGS += -O0 -g3 -DDEBUG_DEV=1
else
CFLAGS += -O3
endif

## General
CPPFLAGS += -c -Wall -ansi -fmessage-length=0
## Dependency files
CPPFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"

ifdef DEBUG
CPPFLAGS += -O0 -g3 -DDEBUG_DEV=1
else
CPPFLAGS += -O3
endif

INCLUDES += -Isrc -Iinc $(patsubst %, -I%/inc, $(REF_PROJECTS))

C_SRCS := $(patsubst %, $(PRJ_LOC)/%.c, $(C_MODULES))
C_OBJS := $(patsubst %, $(MK_CONFIG)/$(PRJ_LOC)/%.o, $(C_MODULES))
C_DEPS := $(patsubst %, $(MK_CONFIG)/$(PRJ_LOC)/%.d, $(C_MODULES))

CPP_SRCS := $(patsubst %, $(PRJ_LOC)/%.cpp, $(CPP_MODULES))
CPP_OBJS := $(patsubst %, $(MK_CONFIG)/$(PRJ_LOC)/%.o, $(CPP_MODULES))
CPP_DEPS := $(patsubst %, $(MK_CONFIG)/$(PRJ_LOC)/%.d, $(CPP_MODULES))

REF_LIBS ?= $(foreach R_LIB, $(REF_PROJECTS), \
             $(R_LIB)/$(MK_CONFIG)/lib$(notdir $(R_LIB)).a)
LDREF_LIBS := $(addprefix -l, \
                  $(patsubst lib%, %, $(notdir $(basename $(REF_LIBS)))))
LDREF_DIRS := $(addprefix -L, $(patsubst %/, %, $(dir $(REF_LIBS))))

LIBES = $(LDREF_DIRS) $(LDREF_LIBS) $(EXT_LIBS)

ifdef DEBUG
LDFLAGS += -g3
endif

ifeq ($(OS),Darwin)
  LDFLAGS += -Wl,-map,$@.map
else
  LDFLAGS += -Wl,-Map,$@.map
endif

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

OBJS = $(CPP_OBJS) $(C_OBJS) $(USER_OBJS)

# Rules

# $(MK_CONFIG)/$(PRJ_LOC)/%.mpi.o : $(PRJ_LOC)/%.mpi.c
#	$(MPICC) $(CFLAGS) $(INCLUDES) $($(*F)_CFLAGS) $(CFLAGS_USER) $(MPICFLAGS_USER) -o"$@" "$<"

$(MK_CONFIG)/$(PRJ_LOC)/%.mpi.o : $(PRJ_LOC)/%.mpi.c
	$(MPICXX) $(CPPFLAGS) $(INCLUDES) $($(*F)_CFLAGS) $(CPFLAGS_USER) $(MPICPPFLAGS_USER) -o"$@" "$<"

$(MK_CONFIG)/$(PRJ_LOC)/%.mpi.o : $(PRJ_LOC)/%.mpi.cpp
	$(MPICXX) $(CPPFLAGS) $(INCLUDES) $($(*F)_CPPFLAGS) $(CPPFLAGS_USER) $(MPICPPFLAGS_USER) -o"$@" "$<"

#$(MK_CONFIG)/$(PRJ_LOC)/%.o : $(PRJ_LOC)/%.c
#    $(MPICC) $(CFLAGS) $(INCLUDES) $($(*F)_CPPFLAGS) $(CPPFLAGS_USER) -o"$@" "$<"

$(MK_CONFIG)/$(PRJ_LOC)/%.o : $(PRJ_LOC)/%.c
	$(MPICXX) $(CPPFLAGS) $(INCLUDES) $($(*F)_CPPFLAGS) $(CPPFLAGS_USER) -o"$@" "$<"

$(MK_CONFIG)/$(PRJ_LOC)/%.o : $(PRJ_LOC)/%.cpp
	$(MPICXX) $(CPPFLAGS) $(INCLUDES) $($(*F)_CPPFLAGS) $(CPPFLAGS_USER) -o"$@" "$<"

