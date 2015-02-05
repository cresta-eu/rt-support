#
# rta-c - message passing communication routines library.
#  Copyright (C) 2014  Sebastian Tunstig
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

# --- macros

-include ../../custom.mk

CURRENT_DIR = $(PWD)

CC=$(MPICC)
CFLAGS = -Wall -I$(SCOTCH_INCLUDE) -L$(SCOTCH_LIB)
LFLAGS = -lscotch -lscotcherr -lm

PROJECTDIR = $(CURRENT_DIR)
OBJECTS = $(PROJECTDIR)/build/proc_map.o $(PROJECTDIR)/build/measurement.o $(PROJECTDIR)/build/topology.o $(PROJECTDIR)/build/graph.o $(PROJECTDIR)/build/test_graph.o $(PROJECTDIR)/build/least_squares.o
EXECUTABLES = $(PROJECTDIR)/bin/proc_map $(PROJECTDIR)/bin/test_graph $(PROJECTDIR)/bin/measurement
PROC_MAP_OBJECTS = $(PROJECTDIR)/build/proc_map.o $(PROJECTDIR)/build/measurement.o $(PROJECTDIR)/build/topology.o $(PROJECTDIR)/build/graph.o $(PROJECTDIR)/build/least_squares.o
TESTGRAPHOBJECTS = $(PROJECTDIR)/build/test_graph.o $(PROJECTDIR)/build/graph.o $(PROJECTDIR)/build/topology.o


# --- targets
all:	lib exes

EXE_CFLAGS = -Wall -I$(SCOTCH_INCLUDE) -I$(PROJECTDIR)/include
EXE_LFLAGS = -L$(SCOTCH_LIB) -L$(PROJECTDIR)/lib -lproc_map -lscotch -lscotcherr -lm

EXE_DIR = evaluation

exes :  $(EXE_DIR)/create_graph \
        $(EXE_DIR)/make_all-to-all_target \
        $(EXE_DIR)/make_manhattan_target \
        $(EXE_DIR)/proc_map_eval \
        $(EXE_DIR)/proc_map_refine_eval \
		$(EXE_DIR)/ping_test \
		$(EXE_DIR)/rand_map_eval

$(EXE_DIR)/create_graph: $(EXE_DIR)/create_graph.o
	$(CC) -o $@ $(EXE_CFLAGS) $(EXE_LFLAGS) $<

$(EXE_DIR)/create_graph.o: $(EXE_DIR)/create_graph.c
	$(CC) -c -o $@ $(EXE_CFLAGS) $<

$(EXE_DIR)/make_all-to-all_target: $(EXE_DIR)/make_all-to-all_target.o
	$(CC) -o $@ $(EXE_CFLAGS) $(EXE_LFLAGS) $<

$(EXE_DIR)/make_all-to-all_target.o: $(EXE_DIR)/make_all-to-all_target.c
	$(CC) -c -o $@ $(EXE_CFLAGS) $<

$(EXE_DIR)/make_manhattan_target: $(EXE_DIR)/make_manhattan_target.o
	$(CC) -o $@ $(EXE_CFLAGS) $(EXE_LFLAGS) $<

$(EXE_DIR)/make_manhattan_target.o: $(EXE_DIR)/make_manhattan_target.c
	$(CC) -c -o $@ $(EXE_CFLAGS) $<

$(EXE_DIR)/proc_map_eval: $(EXE_DIR)/proc_map_eval.o
	$(CC) -o $@ $(EXE_CFLAGS) $(EXE_LFLAGS) $<

$(EXE_DIR)/proc_map_eval.o: $(EXE_DIR)/proc_map_eval.c
	$(CC) -c -o $@ $(EXE_CFLAGS) $<

$(EXE_DIR)/proc_map_refine_eval: $(EXE_DIR)/proc_map_refine_eval.o
	$(CC) -o $@ $(EXE_CFLAGS) $(EXE_LFLAGS) $<

$(EXE_DIR)/proc_map_refine_eval.o: $(EXE_DIR)/proc_map_refine_eval.c
	$(CC) -c -o $@ $(EXE_CFLAGS) $<

$(EXE_DIR)/ping_test: $(EXE_DIR)/ping_test.o
	$(CC) -o $@ $(EXE_CFLAGS) $(EXE_LFLAGS) $<

$(EXE_DIR)/ping_test.o: $(EXE_DIR)/ping_test.c
	$(CC) -c -o $@ $(EXE_CFLAGS) $<

$(EXE_DIR)/rand_map_eval: $(EXE_DIR)/rand_map_eval.o
	$(CC) -o $@ $(EXE_CFLAGS) $(EXE_LFLAGS) $<

$(EXE_DIR)/rand_map_eval.o: $(EXE_DIR)/rand_map_eval.c
	$(CC) -c -o $@ $(EXE_CFLAGS) $<


lib:		$(PROC_MAP_OBJECTS)
			ar rcs $(PROJECTDIR)/lib/libproc_map.a $(PROJECTDIR)/build/*
			cp $(PROJECTDIR)/src/*.h $(PROJECTDIR)/include/
			cp $(SCOTCH_INCLUDE)/* $(PROJECTDIR)/include/
			cp $(SCOTCH_LIB)/* $(PROJECTDIR)/lib/
test:		$(PROC_MAP_OBJECTS)
			$(CC) -o $(PROJECTDIR)/bin/proc_map $(PROC_MAP_OBJECTS) $(LFLAGS) $(CFLAGS)
test_graph:		$(TESTGRAPHOBJECTS)
			$(CC) -o $(PROJECTDIR)/bin/test_graph $(TESTGRAPHOBJECTS)

# --- Object files
$(PROJECTDIR)/build/proc_map.o: $(PROJECTDIR)/src/proc_map.c
			cd $(PROJECTDIR)/build; $(CC) $(CFLAGS) -c ../src/proc_map.c $(LFLAGS)
$(PROJECTDIR)/build/topology.o: $(PROJECTDIR)/src/topology.c
			cd $(PROJECTDIR)/build; $(CC) $(CFLAGS) -c ../src/topology.c
$(PROJECTDIR)/build/graph.o: $(PROJECTDIR)/src/graph.c
			cd $(PROJECTDIR)/build; $(CC) $(CFLAGS) -c ../src/graph.c
$(PROJECTDIR)/build/measurement.o: $(PROJECTDIR)/src/measurement.c
			cd $(PROJECTDIR)/build; $(CC) $(CFLAGS) -c ../src/measurement.c
$(PROJECTDIR)/build/test_graph.o: $(PROJECTDIR)/src/test_graph.c
			cd $(PROJECTDIR)/build; $(CC) $(CFLAGS) -c ../src/test_graph.c
$(PROJECTDIR)/build/least_squares.o: $(PROJECTDIR)/src/least_squares.c
			cd $(PROJECTDIR)/build; $(CC) $(CFLAGS) -c ../src/least_squares.c

# --- remove binary and executable files
clean:
			rm -f $(OBJECTS) $(EXECUTABLES)
