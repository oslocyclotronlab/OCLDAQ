
LIBS		=	-L../lib -ldl -lm -lpthread -lsirius
INCLUDES	= 	-I./ \
				-I../../include \
				-I./event_sort/inc \
				-I./types/inc \
				-I./unpack_build/inc \
				-I./utilities/inc


DEFINES		=	-D_FILE_OFFSET_BITS=64
CC			=	gcc
CXX			=	g++
CFLAGS		=	-Wall -W -O3 -g -fPIC -m64 -pthread $(DEFINES) $(INCLUDES)
CXXFLAGS	=	-Wall -W -O3 -g -fPIC -m64 -std=c++11 -pthread $(DEFINES) $(INCLUDES)


OBJECTS		=	main.o \
			experimentsetup.o \
			XIARoutine.o \
			Histogram1D.o \
			Histogram2D.o \
			Histogram3D.o \
			Histograms.o \
			Parameters.o \
			ParticleRange.o \
			XIA_CFD.o \
			Event_builder.o \
			Unpacker.o \
			Sort_Funct.o \
			spectrum_rw.o

TARGET		=	XIAsort


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: event_sort/src/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: types/src/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: types/src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: unpack_build/src/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: utilities/src/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
