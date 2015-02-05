
OS ?= $(shell uname -s)

ifdef DEBUG
MK_CONFIG ?= debug
endif
MK_CONFIG ?= release

.PHONY : all clean

