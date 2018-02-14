# -*- Makefile -*-

LIBS		= -lm -L/usr/X11R6/lib -lX11 -lXm -lXt -L../lib -lsirius
INCLUDES	= -I../../include -I/usr/X11R6/include
DEFINES 	=
CXX			= g++
CXXFLAGS	= -Wall -W -O3 -g -fPIC $(DEFINES) $(INCLUDES)

OBJECTS		= acq_gui.o acq_storage.o acq_start.o acq_reload.o \
			  acq_stop.o acq_status.o acq_clear.o acq_dump.o acq_init.o \
			  m_engine.o m_sort.o io_xtapp.o

TARGET      = acq_master

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -r $(TARGET) $(OBJECTS)

install:
	#Do nothing... yet...