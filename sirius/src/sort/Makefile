# -*- Makefile -*-

BASEDIR = ../..
include $(BASEDIR)/makefile-head

LIBSIRIUS      = ../lib/libsirius.a
LIBSORT        = libsort.a
LIBOFFLINE     = liboffline.a

DEFINES        += -D_FILE_OFFSET_BITS=64

COMMON_SRCS    = spectrum_rw.cpp sort_format.cpp sort_calib.cpp userutil.cpp
libsort_SRCS   = $(COMMON_SRCS) user_routine_basic.cpp sort_main.cpp
liboffline_SRCS = $(COMMON_SRCS)  sort_offline.cpp

COMMON_OBJS    = $(COMMON_SRCS:%.cpp=%.o)
libsort_OBJS   = $(libsort_SRCS:%.cpp=%.o)
liboffline_OBJS = $(liboffline_SRCS:%.cpp=%.o)

CLEAR_SRCS     = acq_clear.cpp
CLEAR_OBJS     = $(CLEAR_SRCS:%.cpp=%.o)

LIBRARIES      = $(LIBSORT) $(LIBOFFLINE)
OBJECTS        = $(sort $(COMMON_OBJS) $(libsort_OBJS) $(liboffline_OBJS) $(CLEAR_OBJS))

CLEAR          = acq_clear
MAMA2ROOT      = mama2root

CXXFLAGS       += $(RFLAGS)
LIBS           += $(RLIBS)

TARGETS = $(LIBRARIES) $(CLEAR) $(MAMA2ROOT)

all: $(TARGETS)
	$(MAKE) -C user

# only install mama2root and acq_clear, not the libraries
install:
	$(INSTALL_BIN) $(CLEAR) $(MAMA2ROOT) $(INSTALL_DIR)
	$(MAKE) -C user $@

##################################################
# made after the example in "info make eval"
# creates a rule for each library
define LIBRARY_template
$(1): $$($(1:%.a=%)_OBJS)
	@$(ECHO) $(ARMSG) $$@ $$^
	$H $(AR) $$@ $$^
	$H $(RANLIB) $$@
endef
$(foreach lib,$(LIBRARIES),$(eval $(call LIBRARY_template,$(lib))))
##################################################

$(CLEAR): $(CLEAR_OBJS) $(COMMON_OBJS) $(LIBSIRIUS)
	@$(ECHO) $(CXXLMSG) $@
	$H $(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(MAMA2ROOT): $(MAMA2ROOT).cpp
	@$(ECHO) "   [CXX-EXE] $@"
	$H $(CXX) $(CXXFLAGS) -DHAVE_MAIN -o $@ $^ $(LIBS) 

$(LIBSIRIUS):
	@$(MAKE) -C $(shell dirname $(LIBSIRIUS)) $(shell basename $(LIBSIRIUS))

include $(MAKEFILE_TAIL)
