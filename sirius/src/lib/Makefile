# Makefile for lib

LIBS		= -lm
INCLUDES	= -I../../include
DEFINES 	= -D_FILE_OFFSET_BITS=64
CXX			= g++
CXXFLAGS	= -Wall -W -O3 -g -fPIC $(DEFINES) $(INCLUDES)

AR			= ar cr
RANLIB		= ranlib

TARGET		= libsirius.a
OBJECTS		= engine_shm.o net_control.o run_command.o utilities.o sort_spectra.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) $@ $^
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -r $(TARGET) $(OBJECTS)

install:
# nothing to install here