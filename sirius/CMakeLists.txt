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
CPMAddPackage(NAME PackageProject.cmake SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../cmake/PackageProject.cmake)

add_library(sirius STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/src/elog_client.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/elog_crypt.cpp
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

find_package(Threads REQUIRED)
target_link_libraries(sirius PUBLIC m Threads::Threads)

# OpenSSL is optional: with it the elog client can talk HTTPS to the elog
# server, without it entries are sent as plain HTTP (with a warning) and a
# built-in SHA-256 is used for the password hash.
if(APPLE AND NOT OPENSSL_ROOT_DIR)
	find_program(HOMEBREW_EXECUTABLE brew)
	if(HOMEBREW_EXECUTABLE)
		execute_process(
			COMMAND ${HOMEBREW_EXECUTABLE} --prefix openssl@3
			OUTPUT_VARIABLE _sirius_openssl_prefix
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET)
		if(_sirius_openssl_prefix AND EXISTS "${_sirius_openssl_prefix}/include/openssl/ssl.h")
			set(OPENSSL_ROOT_DIR "${_sirius_openssl_prefix}")
		endif()
	endif()
endif()
find_package(OpenSSL 1.1 QUIET)
set(SIRIUS_DEPENDENCIES "Threads")
if(OPENSSL_FOUND)
	message(STATUS "sirius: OpenSSL ${OPENSSL_VERSION} found, elog client supports HTTPS")
	target_link_libraries(sirius PRIVATE OpenSSL::SSL OpenSSL::Crypto)
	target_compile_definitions(sirius PRIVATE SIRIUS_HAVE_OPENSSL=1)
	set(SIRIUS_DEPENDENCIES "Threads;OpenSSL")
else()
	message(STATUS "sirius: OpenSSL not found, elog client uses plain HTTP only")
endif()

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
		DEPENDENCIES "${SIRIUS_DEPENDENCIES}"
)