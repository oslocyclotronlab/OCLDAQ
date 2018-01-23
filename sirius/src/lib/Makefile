# -*- Makefile -*-

BASEDIR = ../..
include $(BASEDIR)/makefile-head

DEFINES        += -D_FILE_OFFSET_BITS=64

LIBSIRIUS      = libsirius.a
TARGETS        = $(LIBSIRIUS)

LIB_OBJECTS    = engine_shm.o net_control.o run_command.o utilities.o sort_spectra.o
OBJECTS        = $(LIB_OBJECTS)

all: $(TARGETS)

install:
# nothing to install here

$(LIBSIRIUS): $(LIB_OBJECTS)
	@$(ECHO) $(ARMSG) $@
	$H $(AR) $@ $^
	$H $(RANLIB) $@

include $(MAKEFILE_TAIL)
