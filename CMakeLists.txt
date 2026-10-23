cmake_minimum_required(VERSION 3.14)
project(T-IOT-911-NAN)

# Project options
option(BUILD_TESTS "Build unit tests" ON)

# Global configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The firmware is built with PlatformIO (platformio.ini); CMake only builds the host tests.
# enable_testing() must be called from the top-level project for ctest to find the tests.
enable_testing()

if(BUILD_TESTS)
    add_subdirectory(test)
endif()
