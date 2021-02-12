cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	master
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

CPMAddPackage(NAME sirius SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../sirius)

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_library(Motif INTERFACE)
target_include_directories(Motif INTERFACE ${MOTIF_INCLUDE_DIR})
target_link_libraries(Motif INTERFACE ${MOTIF_LIBRARIES})
add_library(Motif::Motif ALIAS Motif)

add_executable(master
        src/acq_clear.cpp
        src/acq_dump.cpp
        src/acq_gui.cpp
        src/acq_init.cpp
        src/acq_reload.cpp
        src/acq_start.cpp
        src/acq_status.cpp
        src/acq_stop.cpp
        src/acq_storage.cpp
        src/io_xtapp.cpp
        src/m_engine.cpp
        src/m_sort.cpp
        src/net_client.cpp)

target_include_directories(master
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

target_compile_definitions(master PRIVATE _FILE_OFFSET_BITS=64 BUILD_GUI=1 REMOVE_TRACE=1)
set_target_properties(master PROPERTIES CXX_STANDARD 11)
set_property(TARGET master PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(master PRIVATE sirius::sirius Motif::Motif X11::X11 X11::Xt)