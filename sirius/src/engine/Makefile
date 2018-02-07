# Makefile for engine

LIBS		= -lm -lbz2
#LIBS		+= -lbtp -lpthread -lCAENVME
LIBS		+= -lpthread -lCAENVME
INCLUDES	= -I../../include 
INCLUDES	+= -I/usr/local/SBS/1003/include
#DEFINES 	= -D_FILE_OFFSET_BITS=64 -DLINUX -DSUPPORT_USB_CARD -DBT1003 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE -D_REENTRANT -DDEBUG -DSUPPORT_SBS_CARD
#DEFINES		+= -D_FILE_OFFSET_BITS=64 -DMACOSX=1 -DLINUX
DEFINES		+= -D_FILE_OFFSET_BITS=64 -DMACOSX=1 -DLINUX -lSUPPORT_USB_CARD -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE -D_REENTRANT -DDEBUG
CXX		= g++
CXXFLAGS	= -Wall -W -O3 -g -fPIC $(DEFINES) $(INCLUDES)

ENG_OBJ		= engine.o
USB_OBJ		= sbs_usb_comm.o
VME_OBJ		= usb_vme.o

OBJECTS		= $(ENG_OBJ) $(USB_OBJ) $(VME_OBJ)

TARGETS		= engine usb-engine


all: $(TARGETS)

engine: $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

usb-engine: engine
	ln -s $< $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS) $(OBJECTS)
