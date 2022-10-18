cmake_minimum_required(VERSION 3.21)
project(lighttest)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

set(TARGET_NAME ${PROJECT_NAME})
set(TARGET_SOURCE_FILES "lighttest.cpp")
add_executable(${TARGET_NAME} ${TARGET_SOURCE_FILES})
target_include_directories(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(${TARGET_NAME}
    PRIVATE
    asio::asio
    fmt::fmt
    GTest::gtest
)
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
if (NOT MSVC AND NOT WIN32)
    target_compile_options(${TARGET_NAME} PRIVATE
        "-Wall"
        "-Wextra"
        "-Wconversion"
        "-Wpedantic"
        "-Wshadow"
        "-Werror"
    )
else()
    target_compile_options(${TARGET_NAME} PRIVATE
        "/W4"
        "/WX"
        #"/fsanitize=address"  # Doesn't work without Visual Studio
    )
endif()
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})

