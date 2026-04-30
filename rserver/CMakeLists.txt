cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
        rserver
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

# --- Import tools ----
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/CPM.cmake)
# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info
include(../cmake/CPM.cmake)

CPMAddPackage(
        NAME PackageProject.cmake
        GITHUB_REPOSITORY TheLartians/PackageProject.cmake
        VERSION 1.13.0
)

CPMAddPackage("gh:vetlewi/Histogram@2.3.0")
CPMAddPackage("gh:p-ranav/structopt@0.1.3")
CPMAddPackage("gh:jgaa/logfault@0.8.5")

find_package(ROOT CONFIG REQUIRED)

add_executable(rserver ${CMAKE_CURRENT_LIST_DIR}/rserver.cpp ${CMAKE_CURRENT_LIST_DIR}/ROOTServer.cpp)
target_link_libraries(rserver PRIVATE OCL::Histogram ROOT::Hist ROOT::RIO ROOT::RHTTP structopt::structopt logfault::logfault)