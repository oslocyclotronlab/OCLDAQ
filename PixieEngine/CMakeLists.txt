cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	PixieEngine
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
include(cmake/CPM.cmake)

CPMAddPackage(
		NAME PackageProject.cmake
		GITHUB_REPOSITORY TheLartians/PackageProject.cmake
		VERSION 1.4
)

CPMAddPackage(
		NAME Pixie16
		GITHUB_REPOSITORY vetlewi/Pixie16
		GIT_TAG 1155ff15573edb6f2d53f8da7e4082c7f7aa806b
)

CPMAddPackage(
		NAME INIReader
		GITHUB_REPOSITORY jtilly/inih
		GIT_TAG 1185eac0f0977654f9ac804055702e110bb4da91
)

if ( INIReader_ADDED )
	add_library(INIReader INTERFACE IMPORTED)
	target_include_directories(INIReader INTERFACE $<BUILD_INTERFACE:${INIReader_SOURCE_DIR}>)
endif()

add_library(PixieEngine STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/src/functions.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/writeTerminal.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAControl.cpp
		src/FirmwareMap.cpp src/FirmwareMap.h src/XIAModule.cpp src/XIAModule.h src/XIAParser.cpp)

target_include_directories(PixieEngine
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include/Pixie16
		${CMAKE_CURRENT_SOURCE_DIR}/include/plx
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16)

target_compile_definitions(PixieEngine PUBLIC _FILE_OFFSET_BITS=64 REMOVE_TRACE=1)
set_target_properties(PixieEngine PROPERTIES CXX_STANDARD 11)
set_property(TARGET PixieEngine PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(PixieEngine
		PUBLIC
			Pixie16::Pixie16
		PRIVATE
			INIReader)

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