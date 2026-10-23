# CMake, Build, Tests and CI/CD

This document explains how the build system works with CMake, how to run the tests, and how the CI/CD is configured to generate the artifacts.

## Table of Contents
- [CMake, Build, Tests and CI/CD](#cmake-build-tests-and-cicd)
  - [Table of Contents](#table-of-contents)
  - [1. Project structure](#1-project-structure)
  - [2. CMake files](#2-cmake-files)
    - [2.1. Main CMakeLists.txt file](#21-main-cmakeliststxt-file)
    - [2.2. CMakeLists.txt file in src/](#22-cmakeliststxt-file-in-src)
    - [2.3. CMakeLists.txt file in test/](#23-cmakeliststxt-file-in-test)
  - [3. Building the project](#3-building-the-project)
    - [3.1. Prerequisites](#31-prerequisites)
    - [3.2. Build configuration](#32-build-configuration)
    - [3.3. Compilation](#33-compilation)
  - [4. Running the tests](#4-running-the-tests)
    - [4.1. Unit tests](#41-unit-tests)
    - [4.2. Integration tests](#42-integration-tests)
  - [5. CI/CD configuration](#5-cicd-configuration)
    - [5.1. CI workflow file](#51-ci-workflow-file)
    - [5.2. CI steps](#52-ci-steps)
    - [5.3. Artifact generation](#53-artifact-generation)
  - [6. Useful commands](#6-useful-commands)

## 1. Project structure

The project is organized as follows:

```
conveyor/
├── CMakeLists.txt                  # Main CMake configuration
├── src/
│   └── CMakeLists.txt             # CMake configuration for the source code
├── test/
│   ├── CMakeLists.txt             # Test dependencies and host test target
│   ├── host/
│   │   ├── include/Arduino.h      # Fake Arduino core for the PC build
│   │   ├── fakes/                 # gMock doubles of the HAL interfaces
│   │   └── tests/                 # Unit tests (*_test.cpp)
├── include/                        # Header files
│   └── hal/                       # Hardware abstraction interfaces
├── lib/                           # External libraries
└── workflows/
    └── ci.yml                     # GitHub Actions CI configuration
```

## 2. CMake files

### 2.1. Main CMakeLists.txt file

The `CMakeLists.txt` file at the root of the project configures:

- The project name and version
- The C++ standard used (C++17)
- The build options (tests, coverage, etc.)
- The inclusion of the subdirectories (src/, test/)
- The test configuration with CTest
- The artifact generation

Example configuration:

```cmake
cmake_minimum_required(VERSION 3.10)
project(conveyor VERSION 1.0.0 LANGUAGES CXX)

# C++ standard configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_COVERAGE "Build with coverage support" OFF)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(test)

# Test configuration
enable_testing()
include(CTest)
```

### 2.2. CMakeLists.txt file in src/

This file configures the compilation of the main source code:

- Creation of the main library or executable
- Addition of the source files
- Configuration of the include paths
- Linking with the external libraries

Example:

```cmake
# Create the main executable
add_executable(conveyor
    main.cpp
    conveyor/conveyor.cpp
    # ... other source files
)

# Include paths configuration
target_include_directories(conveyor PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link the libraries
target_link_libraries(conveyor
    pthread
    # ... other libraries
)
```

### 2.3. CMakeLists.txt file in test/

This file configures the host build of the firmware logic and the unit tests:

- Fetches googletest 1.14.0 and ArduinoJson 7.4.3 (`FetchContent`, no system install)
- Builds the firmware logic (`firmware_logic`) for the PC, without the ESP32-only adapters in `src/hal/firmware/`
- Builds the `host_tests` executable and registers its tests with CTest

Example:

```cmake
add_library(firmware_logic STATIC ${HOST_LOGIC_SOURCES})

file(GLOB HOST_TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/host/tests/*.cpp
)
add_executable(host_tests ${HOST_TEST_SOURCES})
target_link_libraries(host_tests PRIVATE firmware_logic gmock_main)
gtest_discover_tests(host_tests)
```

## 3. Building the project

### 3.1. Prerequisites

To build the project, you need:

- CMake (version 3.10 or higher)
- A C++17 compiler (GCC, Clang, MSVC)
- Git (for the dependencies)
- Make or Ninja (for the compilation)

### 3.2. Build configuration

Create a build directory and configure the project:

```bash
mkdir build
cd build
cmake ..
```

To enable the tests:

```bash
cmake -DBUILD_TESTS=ON ..
```

To enable code coverage:

```bash
cmake -DBUILD_COVERAGE=ON ..
```

### 3.3. Compilation

Once configured, build the project:

```bash
cmake --build .
```

Or with Make:

```bash
make
```

## 4. Running the tests

### 4.1. Unit tests

The unit tests are located in `test/host/tests/`. They run on the PC and replace
the hardware with the doubles in `test/host/fakes/`.

To run the tests:

```bash
cd build
ctest
```

Or for a specific test:

```bash
ctest -R ConveyorTest
```

To build only the tests (firmware disabled, as in CI):

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
```

### 4.2. Integration tests

The integration scenarios (StateManager <-> Scheduler <-> Conveyor) are in
`test/host/tests/state_manager_test.cpp` and `test/host/tests/scheduler_test.cpp`.
They run with the unit tests, so there is no separate command.

## 5. CI/CD configuration

### 5.1. CI workflow file

The `workflows/ci.yml` file configures the continuous integration with GitHub Actions.

It contains:

- The triggers (push, pull request)
- The build and test jobs
- The artifact generation
- The deployment

### 5.2. CI steps

The CI performs the following steps:

1. **Checkout**: Retrieve the source code
2. **Install dependencies**: Install CMake, the compiler, etc.
3. **Configuration**: Run CMake
4. **Build**: Compile the project
5. **Tests**: Run the unit and integration tests
6. **Artifact generation**: Create the binaries and archives
7. **Artifact upload**: Make the artifacts available

### 5.3. Artifact generation

The generated artifacts include:

- The main executable (`conveyor`)
- The test executables
- The coverage reports (if enabled)
- The build and test logs

## 6. Useful commands

| Command | Description |
|----------|-------------|
| `cmake ..` | Configure the project |
| `cmake --build .` | Build the project |
| `ctest` | Run all the tests |
| `ctest -V` | Run the tests in verbose mode |
| `ctest -R test_name` | Run a specific test |
| `make clean` | Clean the build files |
| `cmake -DBUILD_TESTS=OFF ..` | Disable the tests |
| `cmake -DBUILD_COVERAGE=ON ..` | Enable code coverage |
