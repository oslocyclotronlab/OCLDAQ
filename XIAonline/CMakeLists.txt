cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	XIAonline
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
		NAME cppzmq
		GITHUB_REPOSITORY zeromq/cppzmq
		VERSION 4.7.1
		OPTIONS "CPPZMQ_BUILD_TESTS OFF"
)

CPMAddPackage(
		NAME readerwriterqueue
		GITHUB_REPOSITORY cameron314/readerwriterqueue
		VERSION 1.0.3)

CPMAddPackage(
		NAME CLI11
		GITHUB_REPOSITORY CLIUtils/CLI11
		VERSION 1.9.1
		OPTIONS 
		  "CLI11_BUILD_DOCS OFF"
		  "CLI11_BUILD_TESTS OFF"
		  "CLI11_BUILD_EXAMPLES OFF"
		  "CLI11_BUILD_EXAMPLES_JSON OFF"
		  "CLI11_SINGLE_FILE_TESTS OFF"
		)

CPMAddPackage(
  NAME cereal
  VERSION 1.3.0
  GITHUB_REPOSITORY USCiLab/cereal
  OPTIONS
    "SKIP_PORTABILITY_TEST ON"
    "JUST_INSTALL_CEREAL ON"
)

FIND_PACKAGE(Threads REQUIRED)

if ( readerwriterqueue_ADDED )
  add_library(readerwriterqueue::readerwriterqueue ALIAS readerwriterqueue)
  target_include_directories(readerwriterqueue INTERFACE
          $<BUILD_INTERFACE:${readerwriterqueue_SOURCE_DIR}>)
endif()

if (NOT sirius_ADDED )
	CPMAddPackage(NAME sirius SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../sirius)
endif()


add_executable(XIAonline
		${CMAKE_CURRENT_SOURCE_DIR}/src/Event_builder.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/experimentsetup.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/publisher.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/Sort_Funct.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum_rw.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/Unpacker.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIA_CFD.cpp)

target_include_directories(XIAonline
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

target_compile_definitions(XIAonline PRIVATE _FILE_OFFSET_BITS=64 BUILD_GUI=1 REMOVE_TRACE=1)
set_target_properties(XIAonline PROPERTIES CXX_STANDARD 11)
set_property(TARGET XIAonline PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(XIAonline PUBLIC sirius::sirius cppzmq readerwriterqueue CLI11::CLI11 cereal Threads::Threads)


string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)
packageProject(
		NAME ${PROJECT_NAME}
		VERSION ${PROJECT_VERSION}
		BINARY_DIR ${PROJECT_BINARY_DIR}
		DEPENDENCIES ""
)