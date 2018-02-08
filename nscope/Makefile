
ROOTINC		=	-I$(shell root-config --nonew --incdir)
ROOTLIBS	=	$(shell root-config --nonew --glibs)


DEFINES		=	-DFIRMWARE_FILE=\"/home/vetlewi/Desktop/nscldaq-ddas/main/nscope/DDASFirmwareVersions.txt\"
INCLUDES	=	-I./configuration -I./booter \
				-I/home/vetlewi/Desktop/Pixie16_software_linux/software/app \
				-I/home/vetlewi/Desktop/Pixie16_software_linux/software/sys \
				-I/home/vetlewi/Desktop/Pixie16_software_linux/software/inc \
				-I/home/vetlewi/Desktop/PlxSdk_7.10/Include
INCLUDES	+=	$(ROOTINC)

LIBS		=	-L/home/vetlewi/Desktop/Pixie16_software_linux/software \
				-L/home/vetlewi/Desktop/PlxSdk_7.10/PlxApi/Library \
				-lPixie16App -lPixie16Sys -lPlxApi
LIBS		+=	$(ROOTLIBS)

CXX			=	g++
CXXFLAGS	=	-g -Wall -W -O3 -m64 -fPIC -std=c++11 $(INCLUDES) $(DEFINES)


OBJECTS		=	AnalogSignal.o \
				ExpertMod.o \
				MaxEvent.o \
				Baseline.o \
				Histogram.o \
				Csra.o \
				ModCsrb.o \
				TrigConfig.o \
				Detector.o \
				displayEdit.o \
				EnergyFilter.o \
				PulseShape.o \
				ScopedT.o \
				Table.o \
				Tau.o \
				TriggerFilter.o \
				CFD.o \
				Popup.o \
				lock.o \
				MultCoincDialogue.o \
				MultiplicityState.o \
				MediaLocator.o \
				TimingParamDialogue.o \
				Main.o \
				nscope.o


TARGET		=	nscope


all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
