cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	sirius
	VERSION 2.0.0
	LANGUAGES CXX C)

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

add_library(sirius STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/src/engine_shm.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/net_control.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/run_command.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/sort_spectra.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/utilities.cpp)

target_include_directories(sirius
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>)

target_compile_definitions(sirius PRIVATE _FILE_OFFSET_BITS=64)
set_target_properties(sirius PROPERTIES CXX_STANDARD 11)
set_property(TARGET sirius PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(sirius PUBLIC m)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
		NAME ${PROJECT_NAME}
		VERSION ${PROJECT_VERSION}
		NAMESPACE ${PROJECT_NAME}
		BINARY_DIR ${PROJECT_BINARY_DIR}
		INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
		INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
		VERSION_HEADER "${VERSION_HEADER_LOCATION}"
		DEPENDENCIES ""
)