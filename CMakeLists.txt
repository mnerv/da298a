cmake_minimum_required(VERSION 3.21)
project(nurture VERSION 0.0.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

add_subdirectory(sky)
add_subdirectory(light)
