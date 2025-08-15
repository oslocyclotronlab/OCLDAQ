cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	XIAengine
	VERSION 2.1.0
	LANGUAGES CXX C)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Options ----

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info
include(../cmake/CPM.cmake)

CPMAddPackage(
		NAME PackageProject.cmake
		GITHUB_REPOSITORY TheLartians/PackageProject.cmake
		VERSION 1.4
)

CPMAddPackage(
	NAME inih
	GITHUB_REPOSITORY jtilly/inih
	GIT_TAG 1185eac0f0977654f9ac804055702e110bb4da91
)

if ( inih_ADDED )
	add_library(inih INTERFACE)
	target_include_directories(inih INTERFACE ${inih_SOURCE_DIR})
	add_library(inih::inih ALIAS inih)
endif()

if (NOT sirius_ADDED )
CPMAddPackage(NAME sirius SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../sirius)
endif()

if (NOT Pixie16_ADDED)
	CPMAddPackage(NAME Pixie16 SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../Pixie16)
endif()

if (NOT firmware_ADDED)
	CPMAddPackage(NAME firmware SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../firmware)
endif()


find_package(Threads REQUIRED)

# To find Qt5: -DQt5_DIR=/opt/homebrew/opt/qt@5/lib/cmake/Qt5
find_package(Qt6 COMPONENTS Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(GUI_PROJECT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/BaselineFilterWidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/BaselineFilterWidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CFDFilterWidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CFDFilterWidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ChannelRegisterTab.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ChannelRegisterTab.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CopySettingsTab.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CopySettingsTab.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/EnergyFilterWidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/EnergyFilterWidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ModuleRegisterTab.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ModuleRegisterTab.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/QDCFilterWidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/QDCFilterWidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingFilterTab.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingFilterTab.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingSettingsWidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingSettingsWidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TriggerFilterWidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TriggerFilterWidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/xiaconfigurator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/xiaconfigurator.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/xiainterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/helpers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/helpers.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/enumerate.h
)

qt_add_executable(XIAengine MANUAL_FINALIZATION ${GUI_PROJECT_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/engine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/functions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/WriteTerminal.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/XIAControl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/xiainterface2.cpp
)

target_include_directories(XIAengine
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/engine/src>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/XIAConfigurator>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/XIAConfigurator>
)

set_target_properties(XIAengine PROPERTIES CXX_STANDARD 20)
set_property(TARGET XIAengine PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(XIAengine PRIVATE NUMBER_OF_CHANNELS=16)

target_link_libraries(XIAengine
	PUBLIC
		Pixie16::Pixie16
		sirius::sirius
		Qt6::Core
		Qt6::Widgets
		inih::inih
		Threads::Threads
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_compile_definitions(XIAengine PRIVATE _FILE_OFFSET_BITS=64 BUILD_GUI=1 REMOVE_TRACE=1 OFFLINE=1)
else()
	target_compile_definitions(XIAengine PRIVATE _FILE_OFFSET_BITS=64 BUILD_GUI=1 REMOVE_TRACE=1)
endif()

add_custom_command(
		TARGET XIAengine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_LIST_DIR}/../template/XIA_Firmware.txt
		${CMAKE_CURRENT_BINARY_DIR}/XIA_Firmware.txt
)

add_custom_command(
		TARGET XIAengine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_LIST_DIR}/../template/pxisys.ini
		${CMAKE_CURRENT_BINARY_DIR}/pxisys.ini
)

add_custom_command(
		TARGET XIAengine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_LIST_DIR}/../template/settings.set
		${CMAKE_CURRENT_BINARY_DIR}/settings.set
)

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(XIAengine)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)