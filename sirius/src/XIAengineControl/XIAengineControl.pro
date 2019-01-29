#-------------------------------------------------
#
# Project created by QtCreator 2018-03-20T16:02:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XIAengineControl
TEMPLATE = app
CONFIG += optimize_full

INCLUDEPATH += ../../include

unix:!macx {
INCLUDEPATH += /opt/xia/current/app \
               /opt/xia/current/sys \
               /opt/xia/current/inc \
               /opt/plx/current/PlxSdk/Include

LIBS += -L/opt/xia/current \
        -L/opt/plx/current/PlxSdk/PlxApi/Library \
        -L../lib
}

macx {
INCLUDEPATH += /Users/vetlewi/Desktop/XIA_stuff/Pixie16_software_linux/software/app \
               /Users/vetlewi/Desktop/XIA_stuff/Pixie16_software_linux/software/sys \
               /Users/vetlewi/Desktop/XIA_stuff/Pixie16_software_linux/software/inc

LIBS += -L/Users/vetlewi/Desktop/OCLDAQ/sirius/src/lib \
        -L/Users/vetlewi/Desktop/XIA_stuff/PlxSdk/PlxApi/Library \
        -L/Users/vetlewi/Desktop/XIA_stuff/Pixie16_software_linux/software/

#DEFINES += OFFLINE TESTGUI
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS = -pthread -D_FILE_OFFSET_BITS=64 -O3 -m64 -fPIC -W -Wall -mtune=native
QMAKE_CXXFLAGS_RELEASE = -pthread -D_FILE_OFFSET_BITS=64 -O3 -m64 -fPIC -W -Wall -mtune=native
LIBS += -lsirius -lPixie16App -lPixie16Sys -lPlxApi -pthread -ldl -lrt

SOURCES += \
        engine.cpp \
        mainwindow.cpp \
        WriteTerminal.cpp \
        XIAControl.cpp \
    functions.cpp

HEADERS += \
        mainwindow.h \
        WriteTerminal.h \
        XIAControl.h \
    Functions.h

FORMS += \
        mainwindow.ui
