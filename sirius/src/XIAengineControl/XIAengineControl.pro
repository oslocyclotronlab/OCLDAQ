#-------------------------------------------------
#
# Project created by QtCreator 2018-03-20T16:02:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XIAengineControl
TEMPLATE = app

INCLUDEPATH += ../../include \
              /home/vetlewi/Desktop/Pixie16_software_linux/software/app \
              /home/vetlewi/Desktop/Pixie16_software_linux/software/sys \
              /home/vetlewi/Desktop/Pixie16_software_linux/software/inc


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -pthread -D_FILE_OFFSET_BITS=64 -O3
LIBS += -L/home/vetlewi/Desktop/OCLDAQ/sirius/src/lib -lsirius -L/home/vetlewi/Desktop/PlxSdk_7.10/PlxApi/Library -L/home/vetlewi/Desktop/Pixie16_software_linux/software/ -lPixie16App -lPixie16Sys -lPlxApi -pthread -ldl

SOURCES += \
        engine.cpp \
        mainwindow.cpp \
        WriteTerminal.cpp \
        XIAControl.cpp

HEADERS += \
        mainwindow.h \
        WriteTerminal.h \
        XIAControl.h

FORMS += \
        mainwindow.ui
