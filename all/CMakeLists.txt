cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(BuildAll LANGUAGES CXX)

include(../cmake/tools.cmake)
include(../cmake/CPM.cmake)


CPMAddPackage(NAME sirius SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../sirius)
CPMAddPackage(NAME plx SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../plx)
CPMAddPackage(NAME Pixie16 SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../Pixie16)

CPMAddPackage(NAME master SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../master)
CPMAddPackage(NAME rupdate SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../rupdate)
CPMAddPackage(NAME XIAengine SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../XIAengine)
CPMAddPackage(NAME XIAonline SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../XIAonline)

#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../rupdate SOURCE_DIR ${CMAKE_BINARY_DIR}/rupdate)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../XIAengine SOURCE_DIR ${CMAKE_BINARY_DIR}/XIAengine)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../XIAonline SOURCE_DIR ${CMAKE_BINARY_DIR}/XIAonline)