# PlatformConfig.cmake - Unified platform configuration for ClientForFrameWork
# This module provides platform detection and configuration for Windows and macOS

# ==============================================================================
# Platform Detection
# ==============================================================================
if(NOT DEFINED PLATFORM_NAME)
    if(APPLE)
        set(PLATFORM_NAME "macOS" CACHE STRING "Target platform name")
    elseif(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM_NAME "Win64" CACHE STRING "Target platform name")
    else()
        message(FATAL_ERROR "Unsupported platform: Only Windows x64 and macOS are supported")
    endif()
endif()

message(STATUS "Configuring for platform: ${PLATFORM_NAME}")

# ==============================================================================
# C++ Standard Configuration
# ==============================================================================
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++17 standard")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "Disable compiler extensions")

# ==============================================================================
# Path Configuration with Environment Variable Support
# ==============================================================================

# Allow environment variables to override default paths
if(DEFINED ENV{QT_ROOT_DIR})
    set(QT_ROOT_DIR "$ENV{QT_ROOT_DIR}" CACHE PATH "Qt installation directory")
    message(STATUS "Using Qt from environment variable: ${QT_ROOT_DIR}")
endif()

if(DEFINED ENV{EXT_DIR})
    set(EXT_DIR "$ENV{EXT_DIR}" CACHE PATH "External libraries directory")
    message(STATUS "Using external libraries from environment variable: ${EXT_DIR}")
endif()

# Default paths (can be overridden by environment variables above)
if(NOT DEFINED QT_ROOT_DIR)
    if(APPLE)
        set(QT_ROOT_DIR "/usr/local/opt/qt" CACHE PATH "Qt installation directory")
    elseif(WIN32)
        set(QT_ROOT_DIR "C:/Qt" CACHE PATH "Qt installation directory")
    endif()
endif()

if(NOT DEFINED EXT_DIR)
    set(EXT_DIR "${CMAKE_SOURCE_DIR}/../ext" CACHE PATH "External libraries directory")
endif()

# ==============================================================================
# Output Directory Configuration
# ==============================================================================

# Base output directory (put under target/ in project root)
set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/../target" CACHE PATH "Base output directory")

# Platform-specific subdirectory
set(TARGET_PLATFORM_DIR "${PLATFORM_NAME}" CACHE STRING "Platform-specific output subdirectory")

# Final output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/${TARGET_PLATFORM_DIR}/bin" CACHE PATH "Runtime output directory")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/${TARGET_PLATFORM_DIR}/bin" CACHE PATH "Library output directory")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/${TARGET_PLATFORM_DIR}/lib" CACHE PATH "Archive output directory")

# Ensure output directories exist
file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(MAKE_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
file(MAKE_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

message(STATUS "Output directories:")
message(STATUS "  Runtime: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Library: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "  Archive: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

# ==============================================================================
# Compiler-Specific Configuration
# ==============================================================================

# Common flags for all compilers
set(COMMON_COMPILE_FLAGS
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
    "$<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
)

# Platform-specific compile definitions
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    # Windows-specific flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
elseif(APPLE)
    # macOS-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# ==============================================================================
# Qt Configuration
# ==============================================================================

# Find Qt components (will be used by subdirectories)
set(QT_COMPONENTS Core Gui Widgets Qml Quick QuickControls2)

message(STATUS "Platform configuration complete")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt Root: ${QT_ROOT_DIR}")
message(STATUS "  Ext Dir: ${EXT_DIR}")

# ==============================================================================
# Export Configuration Variables
# ==============================================================================

# Make variables available to parent scope
set(PLATFORM_NAME "${PLATFORM_NAME}" PARENT_SCOPE)
set(EXT_DIR "${EXT_DIR}" PARENT_SCOPE)
set(QT_ROOT_DIR "${QT_ROOT_DIR}" PARENT_SCOPE)
set(OUTPUT_BASE_DIR "${OUTPUT_BASE_DIR}" PARENT_SCOPE)
set(TARGET_PLATFORM_DIR "${TARGET_PLATFORM_DIR}" PARENT_SCOPE)
set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS}" PARENT_SCOPE)
set(QT_COMPONENTS "${QT_COMPONENTS}" PARENT_SCOPE)