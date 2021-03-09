cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Firmware LANGUAGES CXX)

set(FW_BASE ${CMAKE_CURRENT_SOURCE_DIR}/files)
configure_file(XIA_Firmware.txt.in XIA_Firmware.txt @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})