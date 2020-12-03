cmake_minimum_required(VERSION 3.13)
project(OCLDAQ VERSION 2.0.0 LANGUAGES C CXX)


option(ENABLE_ENGINE "Build XIAengine" ON)
option(ENABLE_RUPDATE "Build rupdate" ON)

##############################################
# Declare dependencies

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)
find_package(ROOT CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Threads REQUIRED)

include(external/cppzmq/libzmq-pkg-config/FindZeroMQ.cmake)
if ( NOT ZeroMQ_FOUND )
    message(FATAL_ERROR "Could not find ZeroMQ libraries")
endif()

##############################################
# Create target and set properties

#############     libSirius     ##############
# libSirius will be a static library that
# several of the other components will link up
# with.

add_library(sirius STATIC
    src/sirius/engine_shm.cpp
    src/sirius/net_control.cpp
    src/sirius/run_command.cpp
    src/sirius/sort_spectra.cpp
    src/sirius/utilities.cpp
)

add_library(${PROJECT_NAME}::sirius ALIAS sirius)

target_compile_definitions(sirius PRIVATE _FILE_OFFSET_BITS=64)
target_compile_options(sirius PRIVATE -fPIC -W -Wall)

target_include_directories(sirius
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/sirius>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sirius
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sirius
)

target_link_libraries(sirius PUBLIC m Threads::Threads)

##############     master     ################
# This is the GUI for starting/stopping runs.

add_executable(master
        src/master/acq_clear.cpp
        src/master/acq_dump.cpp
        src/master/acq_gui.cpp
        src/master/acq_init.cpp
        src/master/acq_reload.cpp
        src/master/acq_start.cpp
        src/master/acq_status.cpp
        src/master/acq_stop.cpp
        src/master/acq_storage.cpp
        src/master/io_xtapp.cpp
        src/master/m_engine.cpp
        src/master/m_sort.cpp
        src/master/net_client.cpp)

target_include_directories(master
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/master
        ${MOTIF_INCLUDE_DIR}
)

target_compile_options(master PRIVATE -Wall -W -fPIC)
target_link_libraries(master
    PUBLIC
        ${PROJECT_NAME}::sirius
        ${MOTIF_LIBRARIES} # !!!!!!!IMPORTANT MOTIF HAS TO BE BEFORE Xt!!!!!!
        X11::X11
        X11::Xt
)

############     XIAonline     ##############
# Online sorting of the data.

add_executable(XIAonline
    src/XIAonline/Event_builder.cpp
    src/XIAonline/experimentsetup.c
    src/XIAonline/main.cpp
    src/XIAonline/Sort_Funct.cpp
    src/XIAonline/spectrum_rw.cpp
    src/XIAonline/Unpacker.cpp
    src/XIAonline/XIA_CFD.cpp
    src/XIAonline/publisher.cpp
    src/XIAonline/publisher.h)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 10) # GCC10 implements jthread!
    target_include_directories(XIAonline
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/XIAonline
            ${CMAKE_CURRENT_SOURCE_DIR}/external/cppzmq
            ${CMAKE_CURRENT_SOURCE_DIR}/external/readerwriterqueue
            ${CMAKE_CURRENT_SOURCE_DIR}/external/cereal/include
    )
    set_target_properties(XIAonline PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO)
else()
    target_include_directories(XIAonline
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/XIAonline
            ${CMAKE_CURRENT_SOURCE_DIR}/external/cppzmq
            ${CMAKE_CURRENT_SOURCE_DIR}/external/readerwriterqueue
            ${CMAKE_CURRENT_SOURCE_DIR}/external/cereal/include
            ${CMAKE_CURRENT_SOURCE_DIR}/external/jthread/source
    )
    set_target_properties(XIAonline PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO)
endif()

target_link_libraries(XIAonline ${PROJECT_NAME}::sirius libzmq)

if ( ENABLE_RUPDATE )
    ###########     rupdate     #############
    # This is the rupdate for viewing hists.

    include("${ROOT_DIR}/RootMacros.cmake")

    root_generate_dictionary(G__rupdate
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rupdate/io_root.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rupdate/rupdate.h
            OPTIONS
                -I${CMAKE_CURRENT_SOURCE_DIR}/include
                -I${CMAKE_CURRENT_SOURCE_DIR}/include/sirius
            LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/rupdate/linkdef.h)

    add_library(rupdate SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rupdate/io_root.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rupdate/rupdate.cpp
            G__rupdate)

    target_compile_definitions(rupdate PUBLIC XIA=1)
    target_compile_options(rupdate PUBLIC -Wall -W -fPIC)

    target_link_libraries(rupdate
        PUBLIC
            ${PROJECT_NAME}::sirius
            ROOT::Core
            ROOT::Gpad
            ROOT::Hist
            ROOT::Gui
    )

    add_executable(rupdate-bin
            src/rupdate/main.cpp)

    target_link_libraries(rupdate-bin PRIVATE rupdate)
    set_target_properties(rupdate-bin PROPERTIES OUTPUT_NAME rupdate)

    set_property(TARGET rupdate PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
    set_property(TARGET rupdate-bin PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)


endif()

    ###########     XIAlibraries     #############
    # This is the XIA libraries from XIA SDK

add_library(xia STATIC
    src/xia/communication.c
    src/xia/i2cm24c64.c
    src/xia/pixie16app.c
    src/xia/pixie16sys.c
    src/xia/tools.c
    src/xia/utilities.c
    src/xia/Aardvark.c
    src/xia/Eep_8000.c
    src/xia/PlxApi.c
    src/xia/PlxApiI2cAa.c)

add_library(${PROJECT_NAME}::xia ALIAS xia)

target_compile_definitions(xia PUBLIC
    PLX_LITTLE_ENDIAN
    PCI_CODE
    PLX_LINUX)

target_compile_options(xia PRIVATE -Wall -pipe -fPIC)

target_include_directories(xia
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/xia>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/xia
        ${CMAKE_CURRENT_SOURCE_DIR}/include/xia)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(xia PUBLIC m dl)
else()
    target_link_libraries(xia PUBLIC m dl rt)
endif()

############     XIAengine     ##############
# This is the XIA readout engine.

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(XIAengine
    src/XIAengine/engine.cpp
    src/XIAengine/functions.cpp
    src/XIAengine/mainwindow.cpp
    src/XIAengine/WriteTerminal.cpp
    src/XIAengine/XIAControl.cpp
    src/XIAengine/mainwindow.ui
)

target_compile_definitions(XIAengine PRIVATE BUILD_GUI=1 _FILE_OFFSET_BITS=64)

target_include_directories(XIAengine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/XIAengine
)

set_target_properties(XIAengine PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

target_link_libraries(XIAengine
    ${PROJECT_NAME}::sirius
    ${PROJECT_NAME}::xia
    Qt5::Widgets)

##########################################
###########  Install commands  ###########

install(TARGETS master XIAonline
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

#install(TARGETS XIAengine
#        RUNTIME DESTINATION bin)

install(TARGETS XIAonline rupdate-bin
        RUNTIME DESTINATION bin)
install(TARGETS rupdate
        LIBRARY DESTINATION lib64)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/librupdate_rdict.pcm ${CMAKE_CURRENT_BINARY_DIR}/librupdate.rootmap
        DESTINATION lib64)
