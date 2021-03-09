RLIBS			= $(shell root-config --glibs)
LIBS			=  -L../sort -lsort -L../lib -lsirius -lm
LIBS			+= $(RLIBS)

RFLAGS			= $(shell root-config --cflags)
INCLUDES		= -I../../include -I../sort
DEFINES			= -D_FILE_OFFSET_BITS=64
CXXFLAGS		= -Wall -W -O3 -g -fPIC $(DEFINES) $(INCLUDES)
CXXFLAGS		+= $(RFLAGS)

CXX				= g++
AR				= ar cr
RANLIB			= ranlib
RM				= rm -f

TARGET			= acq_sort

USER_ROUTINE_SRC = user_routine_root.cpp
UR_OBJ			= $(USER_ROUTINE_SRC:%.cpp=%.o)

OBJECTS			= userroot.o
OBJECTS			+= $(UR_OBJ)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) $(TARGET) $(OBJECTS)
