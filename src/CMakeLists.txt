
# Configuration for ESP32 with the Arduino framework
set(ARDUINO_BOARD "m5stack-grey")
set(ARDUINO_PLATFORM "espressif32")
set(ARDUINO_FRAMEWORK "arduino")

# Path to the Arduino installation
set(ARDUINO_PATH "$ENV{HOME}/Arduino" CACHE PATH "Path to Arduino installation")

# Path configuration
set(ARDUINO_LIBRARY_PATH "${ARDUINO_PATH}/libraries")
set(ARDUINO_TOOLS_PATH "${ARDUINO_PATH}/hardware/espressif/esp32/tools")
set(ARDUINO_CORE_PATH "${ARDUINO_PATH}/hardware/espressif/esp32")

# Compiler configuration
set(CMAKE_C_COMPILER "${ARDUINO_TOOLS_PATH}/xtensa-esp32-elf/bin/xtensa-esp32-elf-gcc")
set(CMAKE_CXX_COMPILER "${ARDUINO_TOOLS_PATH}/xtensa-esp32-elf/bin/xtensa-esp32-elf-g++")

# Project sources
file(GLOB_RECURSE PROJECT_SOURCES
    "*.cpp"
    "*.c"
)

# Create the executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Include configuration
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${ARDUINO_CORE_PATH}/cores/esp32
    ${ARDUINO_CORE_PATH}/variants/m5stack_grey
    ${ARDUINO_LIBRARY_PATH}
    ${M5STACK_PATH}/src
    ${GRBL_PATH}/src
    ${MFRC522_PATH}/src
    ${ARDUINOJSON_PATH}/src
)

# Specific definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ARDUINO=10813
    ESP32
    ARDUINO_ARCH_ESP32
    ARDUINO_M5STACK_GREY
    DOLIBARR_URL="\"${DOLIBARR_URL}\""
    DOLIBARR_API_KEY="\"${DOLIBARR_API_KEY}\""
)

# Compilation options
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
    -std=gnu++17
    -fno-exceptions
    -fno-rtti
    -mlongcalls
    -mtext-section-literals
)

# Linking with the Arduino libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    "-L${ARDUINO_TOOLS_PATH}/xtensa-esp32-elf/lib"
    "-T${ARDUINO_CORE_PATH}/tools/sdk/esp32/ld/esp32.out.ld"
    "-Wl,--start-group"
    "-lstdc++"
    "-lm"
    "-lgcc"
    "-lhal"
    "-lesp32"
    "-Wl,--end-group"
)

# Linker configuration
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-Wl,--gc-sections -Wl,--undefined=uxTopUsedPriority"
    SUFFIX ".elf"
)