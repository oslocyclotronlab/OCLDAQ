# -*- Makefile -*-

BASEDIR = ../..
include $(BASEDIR)/makefile-head


INCLUDES       += $(XINCLUDES)
LIBS           += $(XLIBS)
MASTEROBJS     = acq_gui.o acq_storage.o acq_start.o \
                 acq_stop.o acq_status.o acq_clear.o acq_dump.o acq_init.o \
                 m_engine.o m_sort.o io_xtapp.o

LIBSIRIUS      = ../lib/libsirius.a
ACQMASTER      = acq_master

OBJECTS        = $(MASTEROBJS)
TARGETS        = $(ACQMASTER)

all: $(TARGETS)

install: install-targets

$(ACQMASTER): $(MASTEROBJS) $(LIBSIRIUS)
	@$(ECHO) $(CXXLMSG) $@
	$H $(CXX) $(LIBS) $^ -o $@

$(LIBSIRIUS):
	@$(MAKE) -C $(shell dirname $(LIBSIRIUS)) $(shell basename $(LIBSIRIUS))

include $(MAKEFILE_TAIL)
