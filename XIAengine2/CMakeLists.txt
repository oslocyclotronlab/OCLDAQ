cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
project(
	XIAengine2
	VERSION 3.0.0
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



add_executable(server
	server.cpp
)

add_executable(client
	client.cpp
)

set_target_properties(server PROPERTIES CXX_STANDARD 20)
set_target_properties(client PROPERTIES CXX_STANDARD 20)

target_link_libraries(server PRIVATE sirius::sirius)
target_link_libraries(client PRIVATE sirius::sirius)
