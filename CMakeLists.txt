cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(DeathTracker VERSION 1.0.0)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME} EXTERNALS
    "alphalaneous.badgify:1.0.7"
    "alk.better-touch-prio:1.0.22"
    "dankmeme.argon:2.0.0-alpha.3"
    "geode.node-ids:1.23.3"
)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Badgify/include/Badgify.hpp")
    target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Badgify/include")
endif()

set(ARGON_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../argon")
if (EXISTS "${ARGON_SOURCE_DIR}/include/argon.hpp")
    target_include_directories(${PROJECT_NAME} PRIVATE "${ARGON_SOURCE_DIR}/include")

    if (WIN32)
        set(ARGON_IMPORT_LIBRARY "${ARGON_SOURCE_DIR}/build/dankmeme.argon.lib")
        if (NOT EXISTS "${ARGON_IMPORT_LIBRARY}")
            message(FATAL_ERROR "Argon import library not found at ${ARGON_IMPORT_LIBRARY}; build the local Argon mod first.")
        endif()
        target_link_libraries(${PROJECT_NAME} "${ARGON_IMPORT_LIBRARY}")
    endif()
endif()
