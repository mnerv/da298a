cmake_minimum_required(VERSION 3.21)
project(light VERSION 0.0.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(TARGET_NAME ${PROJECT_NAME})
set(TARGET_SOURCE_FILES
    "helloworld.cpp"
    "vcpkg.json"
)
add_executable(${TARGET_NAME} ${TARGET_SOURCE_FILES})
target_include_directories(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(${TARGET_NAME}
    PRIVATE
    asio::asio
    fmt::fmt
    sky
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

add_subdirectory(tests)