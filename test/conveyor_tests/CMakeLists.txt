file(GLOB TEST_SOURCES
    "conveyor_unit_test.cpp"
    "mock_grbl.h"
    "mock_wire.h"
)

add_executable(conveyor_tests ${TEST_SOURCES})

target_include_directories(conveyor_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(conveyor_tests
    GTest::GTest
    GTest::GMock
)

gtest_discover_tests(conveyor_tests)