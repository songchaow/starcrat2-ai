cmake_minimum_required(VERSION 3.1)

project(CommandCenter)

get_filename_component(SCPARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

set(CMAKE_INCLUDE_PATH ${SCPARENT_DIR}/s2client-api/project/include)
set(CMAKE_LIBRARY_PATH ${SCPARENT_DIR}/s2client-api/project/lib)
# Specify output directories.
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

# Include custom macros to find SC2Api.
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SC2Api REQUIRED)

# Build with c++14 support, required by sc2api.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("src")
