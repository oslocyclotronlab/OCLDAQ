LIBS		=	-L/opt/xia/current \
			-L/opt/plx/current/PlxSdk/PlxApi/Library \
			-L../lib \
			-ldl -lm -lpthread -lPixie16App -lPixie16Sys -lPlxApi -lsirius


INCLUDES	= 	-I/opt/xia/current/inc \
			-I/opt/xia/current/app \
			-I/opt/xia/current/sys \
			-I/opt/plx/PlxSdk/PlxApi/include \
			-I../../include


DEFINES		=	-D_FILE_OFFSET_BITS=64
CXX			=	g++
CXXFLAGS	=	-Wall -W -O3 -g -fPIC -m64 -std=c++11 -pthread $(DEFINES) $(INCLUDES)


OBJECTS		=	engine.o \
			XIAControl.o \
			WriteTerminal.o

TARGET		=	XIAengine


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
