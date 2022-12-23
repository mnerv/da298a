cmake_minimum_required(VERSION 3.21)
project(tests VERSION 0.0.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

find_package(fmt CONFIG REQUIRED)
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

set(TARGET_NAME testrunner)
set(TARGET_SOURCE_FILES
    "mcp_tests.hpp"
    "topo_tests.hpp"
    "utility_tests.hpp"

    "testrunner.cpp"
)
add_executable(${TARGET_NAME} ${TARGET_SOURCE_FILES})
target_include_directories(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(${TARGET_NAME}
    PUBLIC
    fmt::fmt
    GTest::gtest
    sky
)
target_compile_definitions(${TARGET_NAME} PRIVATE ${TARGET_DEFINTIONS})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
target_compile_options(${TARGET_NAME} PRIVATE ${TARGET_OPTIONS})
