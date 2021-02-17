cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	rupdate
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

find_package(ROOT CONFIG REQUIRED)

if (NOT sirius_ADDED )
    CPMAddPackage(NAME sirius SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../sirius)
endif()

include("${ROOT_DIR}/RootMacros.cmake")

root_generate_dictionary(G__rupdate
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io_root.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rupdate.h
    OPTIONS
        -I${CMAKE_CURRENT_SOURCE_DIR}/src
        -I${CMAKE_CURRENT_SOURCE_DIR}/../sirius/include
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/linkdef.h) 

add_library(rupdate SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/src/io_root.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rupdate.cpp
        G__rupdate)

target_compile_definitions(rupdate PUBLIC XIA=1)
target_compile_options(rupdate PUBLIC -Wall -W -fPIC)
target_link_libraries(rupdate PUBLIC sirius::sirius ROOT::Core ROOT::Gpad ROOT::Hist ROOT::Gui)

add_executable(rupdate-bin ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(rupdate-bin PRIVATE rupdate)
set_target_properties(rupdate-bin PROPERTIES OUTPUT_NAME rupdate)

install(TARGETS rupdate-bin DESTINATION bin)

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