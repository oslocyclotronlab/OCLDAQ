#************************************************************
# file: Makefile
# September 2008 Alexander Bürger
#
# Aquisition programs for DAISY,
# to be run on bobcat (RIO2 VME CPU)
#************************************************************


# Configuration
TARGETS = mini-evb #voinov-evb
VMELIB = /usr/src/ces/examples/vme/vlib.o

CXX      = g++
CXXFLAGS = -Wall -W -O2 -g

# some variables, to avoid repetitions
MINI_OBJECTS = mini-evb.o
MINI_SOURCES = $(MINI_OBJECTS:%.o=%.cpp)

VOINOV_OBJECTS = voinov-evb.o
VOINOV_SOURCES = $(VOINOV_OBJECTS:%.o=%.cpp)

COMMON_OBJECTS = evb_caen_atdc.o evb_tpu.o vmeutil.o evb_buffer.o \
	evb_tiger_sbs.o evb_tiger_net.o evb_lecroy_1151n.o evb_mesytec_atdc.o \
	evb_readconfig.o
COMMON_SOURCES = $(COMMON_OBJECTS:%.o=%.cpp)

ALL_OBJECTS = $(MINI_OBJECTS) $(VOINOV_OBJECTS) $(COMMON_OBJECTS)
ALL_SOURCES = $(MINI_SOURCES) $(VOINOV_SOURCES) $(COMMON_SOURCES)

# for automatic dependency files
DEPEND = .depend-
DEPFILES = $(ALL_SOURCES:%.cpp=$(DEPEND)%.dep)

all: $(TARGETS)

mini-evb: $(MINI_OBJECTS) $(COMMON_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(VMELIB) -L/lib/ces -lvme -lnetinet

voinov-evb: $(VOINOV_OBJECTS) $(COMMON_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(VMELIB) -L/lib/ces -lvme -lnetinet

$(ALL_OBJECTS): %.o: %.cpp $(DEPEND)%.dep
	$(CXX) $(CXXFLAGS) -c -o $@ $(@:%.o=%.cpp)

bobtime: bobcat-time.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o $(DEPEND)*.dep

clobber: clean
	rm -f $(TARGET)

# automatically create dependency files
$(DEPFILES): $(DEPEND)%.dep: %.cpp
	$(CXX) -M $< > $@

-include $(DEPFILES)
