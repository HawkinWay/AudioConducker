cmake_minimum_required(VERSION 3.20)

project(
        AudioConducker
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Third party dependencies
include(cmake/Dependencies.cmake)

# Build options
option(BUILD_TESTS "Build tests" ON)

# Main executable
add_executable(AudioConducker 
    src/main.cpp
    src/core/Logger.cpp
    src/core/AudioMonitor.cpp
    src/core/ConfigManager.cpp
    src/core/DuckingEngine.cpp
)

target_include_directories(
    AudioConducker
    PRIVATE
        include
)

target_link_libraries(
    AudioConducker
    PRIVATE
        spdlog::spdlog
)

# Compiler warnings
if(MSVC)
    target_compile_options(AudioConducker PRIVATE /W4)
else ()
    target_compile_options(AudioConducker PRIVATE -Wall -Wextra -Wpedantic)
endif ()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
