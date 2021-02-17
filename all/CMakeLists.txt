cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(BuildAll LANGUAGES CXX)

include(../cmake/tools.cmake)
include(../cmake/CPM.cmake)

#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Pixie16 ${CMAKE_BINARY_DIR}/Pixie16)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../sirius ${CMAKE_BINARY_DIR}/sirius)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../master ${CMAKE_BINARY_DIR}/master)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../rupdate ${CMAKE_BINARY_DIR}/rupdate)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../PixieEngine ${CMAKE_BINARY_DIR}/PixieEngine)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../XIAengine ${CMAKE_BINARY_DIR}/XIAengine)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../XIAonline ${CMAKE_BINARY_DIR}/XIAonline)