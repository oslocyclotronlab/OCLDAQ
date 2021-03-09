cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	XIAengine
	VERSION 2.0.0
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

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


add_executable(XIAengine
		${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/functions.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/WriteTerminal.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAControl.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui)

target_include_directories(XIAengine
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_compile_definitions(XIAengine PRIVATE _FILE_OFFSET_BITS=64 BUILD_GUI=1 REMOVE_TRACE=1 OFFLINE=1)
else()
	target_compile_definitions(XIAengine PRIVATE _FILE_OFFSET_BITS=64 BUILD_GUI=1 REMOVE_TRACE=1)
endif()
set_target_properties(XIAengine PROPERTIES CXX_STANDARD 11)
set_property(TARGET XIAengine PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(XIAengine PUBLIC Pixie16::Pixie16 sirius::sirius Qt5::Widgets inih::inih)

add_custom_command(
		TARGET XIAengine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${firmware_BINARY_DIR}/XIA_Firmware.txt
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

install(TARGETS ${PROJECT_NAME} DESTINATION bin)