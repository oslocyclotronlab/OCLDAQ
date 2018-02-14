TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

ROOTFLAGS = $$system( root-config --cflags )
ROOTLIBS = $$system( root-config --libs )


QMAKE_CXXFLAGS += $$ROOTFLAGS
LIBS += $$ROOTLIBS

INCLUDEPATH +=  ./ \
                event_sort/inc \
                types/inc \
                unpack_build/inc \
                utilities/inc \
                /home/vetlewi/Desktop/OCLDAQ/sirius/include

LIBS += -L/home/vetlewi/Desktop/OCLDAQ/sirius/src/lib -lsirius

SOURCES += \ 
    experimentsetup.c \
    types/src/Histogram1D.cpp \
    types/src/Histogram2D.cpp \
    types/src/Histogram3D.cpp \
    types/src/Histograms.cpp \
    types/src/Parameters.cpp \
    types/src/ParticleRange.cpp \
    types/src/XIA_CFD.c \
    unpack_build/src/Event_builder.cpp \
    unpack_build/src/Unpacker.cpp \
    event_sort/src/XIARoutine.cpp \
    main.cpp \
    utilities/src/spectrum_rw.cpp \
    utilities/src/Sort_Funct.cpp


HEADERS += \
    experimentsetup.h \
    event_sort/inc/UserRoutine.h \
    types/inc/Event.h \
    types/inc/Histogram1D.h \
    types/inc/Histogram2D.h \
    types/inc/Histogram3D.h \
    types/inc/Histograms.h \
    types/inc/Parameters.h \
    types/inc/ParticleRange.h \
    types/inc/XIA_CFD.h \
    unpack_build/inc/Event_builder.h \
    unpack_build/inc/Unpacker.h \
    event_sort/inc/XIARoutine.h \
    utilities/inc/spectrum_rw.h \
    utilities/src/Sort_Funct.h

