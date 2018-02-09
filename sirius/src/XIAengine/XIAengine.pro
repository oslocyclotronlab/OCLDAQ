TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../include

QMAKE_CXXFLAGS += -pthread -D_FILE_OFFSET_BITS=64
LIBS += -L../lib -lsirius -L/home/vetlewi/Desktop/PlxSdk_7.10/PlxApi/Library -L/home/vetlewi/Desktop/Pixie16_software_linux/software/ -lPixie16App -lPixie16Sys -lPlxApi -pthread -ldl

INCLUDEPATH +=  ../Pixie16_software_linux/software/inc \
                ../Pixie16_software_linux/software/app \
                ../Pixie16_software_linux/software/sys


SOURCES += engine.cpp \
    engine_shm.cpp \
    net_control.cpp \
    utilities.cpp \
    WriteTerminal.cpp \
    XIAControl.cpp


HEADERS += \
    debug.h \
    engine_shm.h \
    net_control.h \
    utilities.h \
    WriteTerminal.h \
    XIAControl.h
