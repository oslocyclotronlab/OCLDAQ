# Makefile for rupdate

# Library flags
RLIBS			= $(shell root-config --glibs)
LIBS			= -lm $(RLIBS) -L../lib -lsirius

# Clang/gcc flags
RFLAGS			= $(shell root-config --cflags)
DEFINES			=
INCLUDES		= -I../../include -I../sort
CXXFLAGS		= -Wall -W -O2 -g -fPIC -DXIA=1 $(RFLAGS) $(INCLUDES) $(DEFINES)

# Tools
CC				= gcc
CXX				= g++
RM				= rm -f

# Target biaries
TARGET			= rupdate

# Object files
OBJECTS			= rupdate.o io_root.o


all: $(TARGET)

$(TARGET): $(OBJECTS) rootdict.o
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

rootdict.o: rootdict.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

rootdict.cpp: rupdate.h linkdef.h
	rootcint -f $@ -c $^

clean:
	$(RM) rootdict.cpp rootdict.h rootdict_rdict.pcm rootdict.o
	$(RM) $(TARGET) $(OBJECTS)
