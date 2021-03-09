
RLIBS			= $(shell root-config --glibs)
LIBS			= -lm -L../lib -lsirius
LIBS			+= $(RLIBS)

RFLAGS			= $(shell root-config --cflags)
INCLUDES		= -I../../include
DEFINES			= -D_FILE_OFFSET_BITS=64
CXXFLAGS		= -Wall -W -O3 -g -fPIC $(DEFINES) $(INCLUDES)
CXXFLAGS		+= $(RFLAGS)

CXX				= g++
AR				= ar cr
RANLIB			= ranlib
RM				= rm -f

# Object files needed for all targets
OBJECT_COMMON	= spectrum_rw.o sort_format.o sort_calib.o userutil.o user_routine.o

# Online library
ONLINELIB		= libsort.a
OBJECT_ONLINE	= sort_main.o user_routine_basic.o

# Offline library
OFFLINELIB		= liboffline.a
OBJECT_OFFLINE	= sort_offline.o

# Libraries!
LIB_TARGETS		= $(ONLINELIB) $(OFFLINELIB)

# The acq_clear binary target
CLEAR_APL		= acq_clear
OBJECT_CLEAR	= acq_clear.o

# The mama2root binary target
MAMA2ROOT_APL	= mama2root
OBJECT_M2R		= mama2root.o

APL_TARGETS		= $(CLEAR_APL) $(MAMA2ROOT_APL)

TARGETS			= $(APL_TARGETS) $(LIB_TARGETS)
OBJECTS			= $(OBJECT_COMMON) $(OBJECT_ONLINE) $(OBJECT_OFFLINE) $(OBJECT_CLEAR) $(OBJECT_M2R)


##################################################


all: $(TARGETS)


$(CLEAR_APL): $(OBJECT_CLEAR) $(OBJECT_COMMON)
	$(CXX) -o $@ $^ $(LIBS)

$(MAMA2ROOT_APL): $(OBJECT_M2R)
	$(CXX) -o $@ $^ $(LIBS)

$(ONLINELIB): $(OBJECT_COMMON) $(OBJECT_ONLINE)
	$(AR) $@ $^
	$(RANLIB) $@

$(OFFLINELIB): $(OBJECT_COMMON) $(OBJECT_OFFLINE)
	$(AR) $@ $^
	$(RANLIB) $@

$(OBJECT_M2R): mama2root.cpp
	$(CXX) $(CXXFLAGS) -DHAVE_MAIN -o $@ -c $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) $(OBJECTS) $(TARGETS)