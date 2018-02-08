
ROOTINC		=	-I$(shell root-config --nonew --incdir)
ROOTLIBS	=	$(shell root-config --nonew --glibs)


DEFINES		=	-DFIRMWARE_FILE=\"/home/vetlewi/Desktop/nscldaq-ddas/main/nscope/DDASFirmwareVersions.txt\"
INCLUDES	=	-I/home/vetlewi/Desktop/Pixie16_software_linux/software/app \
				-I/home/vetlewi/Desktop/Pixie16_software_linux/software/sys \
				-I/home/vetlewi/Desktop/Pixie16_software_linux/software/inc \
				-I/home/vetlewi/Desktop/PlxSdk_7.10/Include \
				-I../configuration
INCLUDES	+=	$(ROOTINC)

LIBS		=	-L/home/vetlewi/Desktop/Pixie16_software_linux/software \
				-L/home/vetlewi/Desktop/PlxSdk_7.10/PlxApi/Library \
				-lPixie16App -lPixie16Sys -lPlxApi -lBooter
LIBS		+=	$(ROOTLIBS)

CXX			=	g++
AR			= ar cr
RANLIB		= ranlib
CXXFLAGS	=	-g -Wall -W -O3 -m64 -fPIC -std=c++11 $(INCLUDES) $(DEFINES)



OBJECTS		=	Configuration.o \
				ConfigurationParser.o \
				FirmwareVersionFileParser.o \
				HardwareRegistry.o \
				ModEvtFileParser.o



TARGET		=	libConfiguration.a


all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(AR) $@ $^
	$(RANLIB) $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)