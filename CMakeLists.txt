cmake_minimum_required(VERSION 3.20)

project(
        AudioConducker
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Third party dependencies
include(cmake/Dependencies.cmake)

find_package(PkgConfig REQUIRED)

pkg_check_modules(
	PIPEWIRE
	REQUIRED
    IMPORTED_TARGET
		libpipewire-0.3
)

# Build options
option(BUILD_TESTS "Build tests" OFF)

# Core library
add_library(AudioConduckerCore
    src/audio/ActivityDetector.cpp

    src/core/Logger.cpp
    src/core/AudioMonitor.cpp
    src/core/ConfigManager.cpp
    src/core/DuckingEngine.cpp

    src/platform/pipewire/NodeObserver.cpp
    src/platform/pipewire/PipeWireContext.cpp
    src/platform/pipewire/PipeWireBackend.cpp
)

target_include_directories(
    AudioConduckerCore
    PUBLIC
        include
)

target_link_libraries(
    AudioConduckerCore
    PUBLIC
        spdlog::spdlog
        PkgConfig::PIPEWIRE
)

target_compile_features(
    AudioConduckerCore
    PUBLIC
        cxx_std_20
)


# Main executable
add_executable(AudioConducker 
    src/main.cpp
)

target_link_libraries(
    AudioConducker
    PRIVATE
        AudioConduckerCore
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
