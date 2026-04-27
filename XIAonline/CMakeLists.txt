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
		NAME yaml-cpp
		GITHUB_REPOSITORY jbeder/yaml-cpp
		GIT_TAG 56e3bb550c91fd7005566f19c079cb7a503223cf
)

CPMAddPackage(
		NAME magic_enum
		GITHUB_REPOSITORY Neargye/magic_enum
		VERSION 0.9.6
)

CPMAddPackage("gh:jgaa/logfault@0.8.5")


FIND_PACKAGE(Threads REQUIRED)

if (NOT sirius_ADDED )
	CPMAddPackage(NAME sirius SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../sirius)
endif()

add_library(XIAReader

		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Configuration/ConfigManager.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Configuration/UserConfiguration.cpp

		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Format/event.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Format/XIA_CFD.cpp

		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Tasks/Buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Tasks/Sort.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Tasks/Splitter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Tasks/Trigger.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAReader/Tasks/Unpacker.cpp
)

target_include_directories(XIAReader
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/XIAReader>
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/XIAReader/Configuration>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/XIAReader/Format>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/XIAReader/Tasks>

		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/XIAReader>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/XIAReader/Tools>
)

set_target_properties(XIAReader PROPERTIES CXX_STANDARD 20)
set_property(TARGET XIAReader PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(XIAReader PUBLIC
		yaml-cpp::yaml-cpp
		magic_enum::magic_enum
		logfault::logfault
)

add_executable(XIAonline
		#${CMAKE_CURRENT_SOURCE_DIR}/src/Calib.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
		#${CMAKE_CURRENT_SOURCE_DIR}/src/Sort_Funct.cpp
		#${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum_rw.cpp
)

target_include_directories(XIAonline
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

target_compile_definitions(XIAonline PRIVATE _FILE_OFFSET_BITS=64 BUILD_GUI=1 REMOVE_TRACE=1)
set_target_properties(XIAonline PROPERTIES CXX_STANDARD 20)
set_property(TARGET XIAonline PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(XIAonline PUBLIC XIAReader sirius::sirius Threads::Threads logfault::logfault)


install(TARGETS ${PROJECT_NAME} DESTINATION bin)