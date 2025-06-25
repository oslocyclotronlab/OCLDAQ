cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	XIAreadout
	VERSION 1.0.0
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


add_executable(XIAreadout
		${CMAKE_CURRENT_SOURCE_DIR}/Readout.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/functions.cpp)
target_link_libraries(XIAreadout PUBLIC Pixie16::Pixie16 inih::inih)
target_compile_definitions(XIAreadout PRIVATE _FILE_OFFSET_BITS=64)
set_target_properties(XIAreadout PROPERTIES CXX_STANDARD 11)
set_property(TARGET XIAreadout PROPERTY POSITION_INDEPENDENT_CODE ON)
