cmake_minimum_required(VERSION 3.1)

project(CommandCenter)

get_filename_component(SCPARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

set(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/s2client-api/include)
if(UNIX)
    set(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/s2client-api/lib_linux)
endif(UNIX)
if(WIN32)
    set(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/s2client-api/lib_win32)
endif(WIN32)
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
