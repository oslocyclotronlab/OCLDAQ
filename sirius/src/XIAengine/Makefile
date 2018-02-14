

#LIBS		=	-L/path/to/XIA/SDK -L/path/to/PXI/SDK -lPixie16App -lPixie16Sys -lPxiApi -ldl -lm -lpthread
#INCLUDES	=	-I/path/to/xia/inc -I/path/to/XIA/SDK/app -I/path/to/XIA/SDK/sys -I/path/to/PXI/include -I../../include

LIBS		=	-L/home/vetlewi/Desktop/Pixie16_software_linux/software \
				-L/home/vetlewi/Desktop/PlxSdk_7.10/PlxApi/Library \
				-L../lib \
				-ldl -lm -lpthread -lPixie16App -lPixie16Sys -lPlxApi -lsirius


INCLUDES	= 	-I/home/vetlewi/Desktop/Pixie16_software_linux/software/inc \
				-I/home/vetlewi/Desktop/Pixie16_software_linux/software/app \
				-I/home/vetlewi/Desktop/Pixie16_software_linux/software/sys \
				-I/home/vetlewi/Desktop/PlxSdk_7.10/include \
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
