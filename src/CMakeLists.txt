set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# All the source files for the bot.
file(GLOB BOT_SOURCES "*.cpp" "*.h" "*.hpp")


if(NOT BUILD_LIBRARY)
    list(REMOVE_ITEM BOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/PythonObjects.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/MicroMachine.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/MicroMachine.h")
endif ()

# Enable compilation of the SC2 version of the bot.
add_definitions(-DSC2API)

include_directories(SYSTEM "${SC2Api_INCLUDE_DIRS}")

# Show more warnings at compiletime.
if (MSVC)
    add_definitions(-DNOMINMAX)
    # FIXME: put Windows specific options here.
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
        set(MSVC_RUNTIME "static")
    endif()

    # Set compiler options.
    set(variables
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
        message(STATUS
          "MSVC -> forcing use of statically-linked runtime."
        )
        foreach(variable ${variables})
            if(${variable} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
            endif()
        endforeach()
    else()
        message(STATUS
            "MSVC -> forcing use of dynamically-linked runtime."
        )
        foreach(variable ${variables})
            if(${variable} MATCHES "/MT")
                string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
            endif()
        endforeach()
    endif()
else ()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
endif ()
if(BUILD_LIBRARY)
    # Create the library
    add_definitions(-DCOMPILE_TO_LIBRARY)
    python_add_module(micromachine ${BOT_SOURCES})
    target_link_libraries(micromachine ${SC2Api_LIBRARIES} ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} )
    if (APPLE)
        target_link_libraries(micromachine "-framework Carbon")
    endif ()
    # Linux specific.
    if (UNIX AND NOT APPLE)
        target_link_libraries(micromachine pthread dl)
    endif ()

    install (TARGETS micromachine DESTINATION lib)
    install (FILES ${PROJECT_SOURCE_DIR}/boost-python/lib_linux/libboost_python36.so.1.68.0 DESTINATION lib)
    install (FILES ${PROJECT_SOURCE_DIR}/BotConfig.txt DESTINATION script)
    install (DIRECTORY script DESTINATION . FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

else()
    # Create the executable.
    add_executable(CommandCenter ${BOT_SOURCES})
    target_link_libraries(CommandCenter ${SC2Api_LIBRARIES})
    if (APPLE)
        target_link_libraries(CommandCenter "-framework Carbon")
    endif ()
    # Linux specific.
    if (UNIX AND NOT APPLE)
        target_link_libraries(CommandCenter pthread dl)
    endif ()
endif ()


