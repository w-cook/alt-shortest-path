cmake_minimum_required(VERSION 3.20)

project(alt_shortest_path LANGUAGES C)

# Require standard C17 without compiler-specific language extensions.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Generate compilation information for clangd and other development tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable CTest integration for future automated tests.
include(CTest)

# Build the ALT library.
add_library(alt_shortest_path STATIC
    src/graph.c
)

# Make the public headers available to the library and its consumers.
target_include_directories(alt_shortest_path
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
