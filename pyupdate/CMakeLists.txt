cmake_minimum_required(VERSION 3.18)
project(PyUpdate VERSION 0.0.1 LANGUAGES C CXX)

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
        XIAengine
        VERSION 2.0.1
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

if (NOT sirius_ADDED )
    CPMAddPackage(NAME sirius SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../sirius)
endif()

find_package(Threads REQUIRED)

add_library(PyUpdate )