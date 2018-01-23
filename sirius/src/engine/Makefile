# -*- Makefile -*-

BASEDIR = ../..
include $(BASEDIR)/makefile-head

DEFINES        += -D_FILE_OFFSET_BITS=64

LIBSIRIUS      = ../lib/libsirius.a
ENGINE_OBJECTS = engine.o sbs_usb_comm.o
TARGETS        = engine usb-engine

ifeq ($(shell echo -n `hostname`),tiger)
# This is required to identify what SBS Software Product is being used
DEFINES        += -DBT1003  -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE -D_REENTRANT -DDEBUG -DSUPPORT_SBS_CARD
INCLUDES       += -I/usr/local/SBS/1003/include
# LIBS           += =-L$(BTINCLUDE) not necessary, lib is in /usr/lib somehow
LIBS           += -lbtp -lpthread 

DEFINES        += -DLINUX -DNDEBUG -DSUPPORT_USB_CARD
LIBS           += -lCAENVME -lbz2
ENGINE_OBJECTS += usb_vme.o
endif

OBJECTS        = $(ENGINE_OBJECTS)

all: $(TARGETS)

install:
	$(INSTALL_BIN) engine $(INSTALL_DIR)
	( cd $(INSTALL_DIR) && ln -sf engine usb-engine )

engine: $(ENGINE_OBJECTS) $(LIBSIRIUS)
	@$(ECHO) $(CXXLMSG) $@
	$H $(CXX) $(CXXFLAGS) $(ELIBS) $(LIBS) $^ -o $@

usb-engine: engine
	ln -s $< $@

$(LIBSIRIUS):
	@$(MAKE) -C $(shell dirname $(LIBSIRIUS)) $(shell basename $(LIBSIRIUS))

include $(MAKEFILE_TAIL)
