cmake_minimum_required(VERSION 3.1)

project(CommandCenter)

set(CMAKE_INSTALL_PREFIX installation)
set(BUILD_LIBRARY TRUE)

FIND_PACKAGE(PythonInterp 3.6 REQUIRED)

# set these with your own boost.
# set(BOOST_ROOT "/media/songchaow/DATA/Codes/StarCraftII/boost_1_68_0/installation_linux")
# set(Boost_NO_SYSTEM_PATHS ON)
# set(BOOST_INCLUDEDIR "/home/songchaow/Codes/boost_1_68_0/build/include")
# set(BOOST_LIBRARYDIR "/home/songchaow/Codes/boost_1_68_0/build/lib")

if (PYTHONINTERP_FOUND)
  if (UNIX AND NOT APPLE)
    if (PYTHON_VERSION_MAJOR EQUAL 3)
        FIND_PACKAGE(Boost COMPONENTS python)
	#FIND_PACKAGE(Boost COMPONENTS python)
        FIND_PACKAGE(PythonInterp 3)
        FIND_PACKAGE(PythonLibs 3 REQUIRED)
    else()
        FIND_PACKAGE(Boost COMPONENTS python)
        FIND_PACKAGE(PythonInterp)
        FIND_PACKAGE(PythonLibs REQUIRED)
    endif()
  else()
    if (PYTHON_VERSION_MAJOR EQUAL 3)
        FIND_PACKAGE(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
        FIND_PACKAGE(PythonInterp 3)
        FIND_PACKAGE(PythonLibs 3 REQUIRED)
    else()
        FIND_PACKAGE(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
        FIND_PACKAGE(PythonInterp)
        FIND_PACKAGE(PythonLibs REQUIRED)
    endif()
  endif()
else()
    message("Python not found")
endif()

message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")

set(Boost_LIBRARIES "${PROJECT_SOURCE_DIR}/boost-python/lib_linux/libboost_python36.so.1.68.0")
set(Boost_INCLUDE_DIRS "/home/songchaow/Codes/boost_1_68_0")
set(PYTHON_LIBRARIES "${PROJECT_SOURCE_DIR}/boost-python/lib_linux/libpython3.6m.so.1.0")

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS}  ) 

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} )
get_filename_component(SCPARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/s2client-api/include)
if(UNIX)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/s2client-api/lib_linux)
endif(UNIX)
if(WIN32)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/s2client-api/lib_win32)
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
