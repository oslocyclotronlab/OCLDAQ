cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	XIAConfigurator
	VERSION 0.0.1
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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(XIAConfigurator_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/include/XIAConfigurator/xiaconfigurator.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/BaselineFilterWidget.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CFDFilterWidget.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ChannelRegisterTab.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CopySettingsTab.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/EnergyFilterWidget.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ModuleRegisterTab.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/QDCFilterWidget.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingFilterTab.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingSettingsWidget.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TriggerFilterWidget.h
)

add_library(XIAConfigurator STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/BaselineFilterWidget.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CFDFilterWidget.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ChannelRegisterTab.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/CopySettingsTab.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/EnergyFilterWidget.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/helpers.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/ModuleRegisterTab.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/QDCFilterWidget.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingFilterTab.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TimingSettingsWidget.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/TriggerFilterWidget.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/xiaconfigurator.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/xiainterface.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator/xiainterface_remote.cpp
		${XIAConfigurator_HEADERS}
)

add_library(XIA::XIAConfigurator ALIAS XIAConfigurator)


target_include_directories(XIAConfigurator
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/XIAConfigurator>
		$<INSTALL_INTERFACE:include/XIAConfigurator>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src/XIAConfigurator
)

set_target_properties(XIAConfigurator PROPERTIES CXX_STANDARD 20 AUTOMOC ON)
set_property(TARGET XIAConfigurator PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(XIAConfigurator PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)
target_compile_definitions(XIAConfigurator PRIVATE NUMBER_OF_CHANNELS=16)

add_executable(xia_configurator_client
	${CMAKE_CURRENT_SOURCE_DIR}/src/xiaconfigurator_main.cpp
)
target_include_directories(xia_configurator_client PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include/XIAConfigurator
)
target_link_libraries(xia_configurator_client PRIVATE XIAConfigurator)
set_target_properties(xia_configurator_client PROPERTIES CXX_STANDARD 20 AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
set_target_properties(xia_configurator_client PROPERTIES OUTPUT_NAME xiaconfigurator)
install(TARGETS xia_configurator_client RUNTIME DESTINATION bin)
