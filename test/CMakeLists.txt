enable_testing()

include(FetchContent)

# Test dependencies, fetched at configure time (no system install needed)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_Declare(arduinojson
    URL https://github.com/bblanchon/ArduinoJson/archive/refs/tags/v6.21.5.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest arduinojson)

# Firmware logic, built for the host. Hardware-specific files (src/hal/firmware)
# are deliberately not listed: they only compile for the ESP32.
set(HOST_LOGIC_SOURCES
    ${CMAKE_SOURCE_DIR}/src/error.cpp
    ${CMAKE_SOURCE_DIR}/src/globals.cpp
    ${CMAKE_SOURCE_DIR}/src/api/DolibarrAPI.cpp
    ${CMAKE_SOURCE_DIR}/src/check_connect/check_connect.cpp
    ${CMAKE_SOURCE_DIR}/src/conveyor/conveyor.cpp
    ${CMAKE_SOURCE_DIR}/src/init_position/init_position.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/state_manager/state_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/sorter/addresser/addresser.cpp
    ${CMAKE_SOURCE_DIR}/src/sorter/main_sorter.cpp
    ${CMAKE_SOURCE_DIR}/src/sorter/rfid_reader/reader.cpp
    ${CMAKE_SOURCE_DIR}/src/sorter/servo_sorter/servo_sorter.cpp
)

add_library(firmware_logic STATIC ${HOST_LOGIC_SOURCES})

target_include_directories(firmware_logic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/hal
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/host/include
)

target_link_libraries(firmware_logic PUBLIC ArduinoJson)

target_compile_definitions(firmware_logic PUBLIC
    DOLIBARR_URL="\"http://localhost/api/index.php\""
    DOLIBARR_API_KEY="\"test-api-key\""
)

target_compile_options(firmware_logic PRIVATE -Wall -Wextra -Werror)

# Unit tests
file(GLOB HOST_TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/host/tests/*.cpp
)

add_executable(host_tests ${HOST_TEST_SOURCES})

target_include_directories(host_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/host/fakes
)

target_link_libraries(host_tests PRIVATE
    firmware_logic
    gmock_main
)

include(GoogleTest)
gtest_discover_tests(host_tests)
