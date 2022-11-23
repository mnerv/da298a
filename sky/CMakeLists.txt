cmake_minimum_required(VERSION 3.21)
project(sky VERSION 0.0.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

find_package(GTest CONFIG REQUIRED)

if (NOT MSVC)
    set(TARGET_OPTIONS
        "-Wall"
        "-Wextra"
        "-Wconversion"
        "-Wpedantic"
        "-Wshadow"
        "-Werror"
    )
else()
    set(TARGET_OPTIONS
        "/W4"
        "/WX"
    )
endif()

set(TARGET_NAME ${PROJECT_NAME})
set(TARGET_SOURCE_FILES
    "mcp.hpp"
    "sky.hpp"
    "topo.hpp"

    "mcp.cpp"
    "topo.cpp"

    "vcpkg.json"
)
add_library(${TARGET_NAME} STATIC ${TARGET_SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
target_compile_options(${TARGET_NAME} PRIVATE ${TARGET_OPTIONS})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})
