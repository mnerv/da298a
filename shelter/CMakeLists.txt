cmake_minimum_required(VERSION 3.21)
project(shelter VERSION 0.0.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Threads REQUIRED)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

set(TARGET_DEFINITIONS "ASIO_STANDALONE")
if (NOT MSVC)
    string(JOIN " " TARGET_DEFINTIONS
        ${TARGET_DEFINTIONS}
    )
    set(TARGET_OPTIONS
        "-Wall"
        "-Wextra"
        "-Wconversion"
        "-Wpedantic"
        "-Wshadow"
        "-Werror"
    )
else()
    string(JOIN " " TARGET_DEFINTIONS
        ${TARGET_DEFINTIONS}
        "_WIN32_WINNT=0x0A00"
    )
    set(TARGET_OPTIONS
        "/W4"
        "/WX"
    )
endif()

if (APPLE)
    set(TARGET_LIBRARIES
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
elseif (LINUX)
    set(TARGET_LIBRARIES
        "-dl"
        "-m"
        "-GL"
        "-X11"
    )
elseif (WIN32)
    set(TARGET_LIBRARIES "OpenGL32.lib")
endif()
if (NOT DEFINED TARGET_LIBRARIES)
    message(FATAL_ERROR "TARGET_LIRBARIES not defined!")
endif()

set(TARGET_NAME shelter)
set(TARGET_SOURCE_FILES
    "shelter/buffer.hpp"
    "shelter/camera.hpp"
    "shelter/common.hpp"
    "shelter/graphics_context.hpp"
    "shelter/shelter.hpp"
    "shelter/utility.hpp"
    "shelter/renderer.hpp"
    "shelter/shader.hpp"
    "shelter/window.hpp"

    "shelter/buffer.cpp"
    "shelter/camera.cpp"
    "shelter/graphics_context.cpp"
    "shelter/renderer.cpp"
    "shelter/shader.cpp"
    "shelter/window.cpp"
)
add_library(${TARGET_NAME} OBJECT ${TARGET_SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC "${PROJECT_SOURCE_DIR}" PRIVATE "${PROJECT_SOURCE_DIR}/shelter")
target_link_libraries(${TARGET_NAME}
    PUBLIC
    Threads::Threads
    asio::asio
    fmt::fmt

    glfw
    glad::glad
    glm::glm
    imgui::imgui

    ${TARGET_LIBRARIES}
)
target_compile_definitions(${TARGET_NAME} PRIVATE ${TARGET_DEFINTIONS})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
target_compile_options(${TARGET_NAME} PRIVATE ${TARGET_OPTIONS})

set(TARGET_NAME sandbox)
set(TARGET_SOURCE_FILES
    "flicker.hpp"
    "sandbox.cpp"
    "vcpkg.json"
)
add_executable(${TARGET_NAME} ${TARGET_SOURCE_FILES})
target_include_directories(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(${TARGET_NAME}
    PRIVATE
    sky
    shelter
    ${TARGET_LIBRARIES}
)
target_compile_definitions(${TARGET_NAME} PRIVATE ${TARGET_DEFINTIONS})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
target_compile_options(${TARGET_NAME} PRIVATE ${TARGET_OPTIONS})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})
