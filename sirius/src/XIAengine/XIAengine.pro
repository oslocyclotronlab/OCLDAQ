TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../include

DEFINES += MULTITHREAD=1

QMAKE_CXXFLAGS += -pthread -D_FILE_OFFSET_BITS=64
LIBS += -L/home/vetlewi/Desktop/OCLDAQ/sirius/src/lib -lsirius -L/home/vetlewi/Desktop/PlxSdk_7.10/PlxApi/Library -L/home/vetlewi/Desktop/Pixie16_software_linux/software/ -lPixie16App -lPixie16Sys -lPlxApi -pthread -ldl

INCLUDEPATH +=  /home/vetlewi/Desktop/Pixie16_software_linux/software/inc \
                /home/vetlewi/Desktop/Pixie16_software_linux/software/app \
                /home/vetlewi/Desktop/Pixie16_software_linux/software/sys


SOURCES += engine.cpp \
    WriteTerminal.cpp \
    XIAControl.cpp


HEADERS += \
    WriteTerminal.h \
    XIAControl.h
