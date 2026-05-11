cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	XIAengine
	VERSION 2.1.0
	LANGUAGES CXX C)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
include(../cmake/CPM.cmake)

CPMAddPackage(
		NAME PackageProject.cmake
		GITHUB_REPOSITORY TheLartians/PackageProject.cmake
		VERSION 1.4
)

CPMAddPackage(
	NAME nlohmann_json
	VERSION 3.11.3
	GITHUB_REPOSITORY nlohmann/json
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

find_package(Threads REQUIRED)

add_library(plx STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/Aardvark.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/Eep_8000.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/I2cAaUsb.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/MdioSpliceUsb.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/PlxApi.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/PlxApiDirect.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/SdbComPort.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx/SpiFlash.c
)

target_compile_definitions(plx PUBLIC PLX_LITTLE_ENDIAN PLX_64BIT GCC PCI_CODE PLX_LINUX)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(plx PUBLIC dl m)
else()
	target_link_libraries(plx PUBLIC dl rt m)
endif()


set_property(TARGET plx PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(plx::plx ALIAS plx)
target_include_directories(plx
		PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/plx>
		$<INSTALL_INTERFACE:include/plx>
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src/plx)

add_library(Pixie16 STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/communication.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/i2cm24c64.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/pixie16app.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/pixie16app_globals.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/pixie16sys.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/pixie16sys_globals.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/tools.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16/utilities.c
)

target_include_directories(Pixie16
		PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/Pixie16>
		$<INSTALL_INTERFACE:include/Pixie16>
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src/Pixie16)

target_compile_definitions(Pixie16 PUBLIC PLX_LITTLE_ENDIAN PLX_64BIT GCC PCI_CODE PLX_LINUX)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(Pixie16 PUBLIC dl m plx::plx)
else()
	target_link_libraries(Pixie16 PUBLIC dl rt m plx::plx)
endif()
add_library(XIA::Pixie16 ALIAS Pixie16)
set_property(TARGET Pixie16 PROPERTY POSITION_INDEPENDENT_CODE ON)

add_executable(XIAengine
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/engine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/functions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/WriteTerminal.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/XIAControl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/xiainterface_base.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/xiainterface2.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/engine_json_rpc.cpp
)

target_include_directories(XIAengine
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/engine>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

set_target_properties(XIAengine PROPERTIES CXX_STANDARD 20)
set_property(TARGET XIAengine PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(XIAengine PRIVATE NUMBER_OF_CHANNELS=16)

target_link_libraries(XIAengine
	PRIVATE
		XIA::Pixie16
		sirius::sirius
		nlohmann_json::nlohmann_json
		inih::inih
		Threads::Threads
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_compile_definitions(XIAengine PRIVATE _FILE_OFFSET_BITS=64 REMOVE_TRACE=1 OFFLINE=1)
else()
	target_compile_definitions(XIAengine PRIVATE _FILE_OFFSET_BITS=64 REMOVE_TRACE=1)
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

install(TARGETS XIAengine RUNTIME DESTINATION bin)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../XIAConfigurator ${CMAKE_CURRENT_BINARY_DIR}/XIAConfigurator_sub)
