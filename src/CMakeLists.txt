# Configuration pour ESP32 avec Arduino framework
set(ARDUINO_BOARD "m5stack-grey")
set(ARDUINO_PLATFORM "espressif32")
set(ARDUINO_FRAMEWORK "arduino")

# Trouver le framework Arduino
find_package(Arduino REQUIRED)

# Sources du projet
file(GLOB_RECURSE PROJECT_SOURCES
    "*.cpp"
    "*.c"
)

# Création de l'exécutable
arduino_add_executable(
    ${PROJECT_NAME}
    ${PROJECT_SOURCES}
    BOARD=${ARDUINO_BOARD}
    PLATFORM=${ARDUINO_PLATFORM}
    FRAMEWORK=${ARDUINO_FRAMEWORK}
)

# Configuration des includes avec vos bibliothèques
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${ARDUINO_INCLUDE_DIRS}
    ${M5STACK_PATH}/src
    ${GRBL_PATH}/src
    ${MFRC522_PATH}/src
    ${ARDUINOJSON_PATH}/src
)

# Définitions spécifiques
target_compile_definitions(${PROJECT_NAME} PRIVATE
    DOLIBARR_URL="\"${DOLIBARR_URL}\""
    DOLIBARR_API_KEY="\"${DOLIBARR_API_KEY}\""
)

# Options de compilation
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
)