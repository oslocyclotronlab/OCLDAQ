# -*- Makefile -*-

BASEDIR = ../..
include $(BASEDIR)/makefile-head

LIBS           += $(RLIBS)
CXXFLAGS       += -I../sort $(RFLAGS)

LIBSIRIUS      = ../lib/libsirius.a
RUPDATE        = rupdate
TARGETS        = $(RUPDATE)

RUPDATE_SRCS   = rupdate.cpp io_root.cpp
RUPDATE_OBJS   = $(RUPDATE_SRCS:%.cpp=%.o)

OBJECTS        = $(sort $(RUPDATE_OBJS))

all: $(TARGETS)

install: install-targets

$(RUPDATE): $(RUPDATE_OBJS) rootdict.o $(LIBSIRIUS)
	@$(ECHO) $(CXXLMSG) $@
	$H $(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(LIBSIRIUS):
	@$(MAKE) -C $(shell dirname $(LIBSIRIUS)) $(shell basename $(LIBSIRIUS))

rootdict.cpp: rupdate.h linkdef.h
	@$(ECHO) "      [CINT] $<"
	$H rootcint -f $@ -c $^

clean: rclean
rclean:
	$(RM) rootdict.cpp rootdict.h

include $(MAKEFILE_TAIL)
