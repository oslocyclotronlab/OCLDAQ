cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	plx
	VERSION 8.0.0
	LANGUAGES C)

# ---- Options ----

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
# see https://github.com/TheLartians/CPM.cmake for more info
include(../cmake/CPM.cmake)

CPMAddPackage(
		NAME PackageProject.cmake
		GITHUB_REPOSITORY TheLartians/PackageProject.cmake
		VERSION 1.4
)


add_library(plx STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/Aardvark.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Eep_8000.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/I2cAaUsb.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/MdioSpliceUsb.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/PlxApi.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/PlxApiDirect.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/SdbComPort.c)

target_include_directories(plx
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(plx PUBLIC PLX_LITTLE_ENDIAN PLX_64BIT GCC PCI_CODE PLX_LINUX)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(plx PUBLIC dl m)
else()
	target_link_libraries(plx PUBLIC dl rt m)
endif()


set_property(TARGET plx PROPERTY POSITION_INDEPENDENT_CODE ON)

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