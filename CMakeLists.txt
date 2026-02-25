cmake_minimum_required(VERSION 3.14)
project(T-IOT-911-NAN)

# Options du projet
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_FIRMWARE "Build firmware for M5Stack" ON)

# Configuration globale
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Variables d'environnement
set(DOLIBARR_URL "$ENV{DOLIBARR_URL}" CACHE STRING "Dolibarr API URL")
set(DOLIBARR_API_KEY "$ENV{DOLIBARR_API_KEY}" CACHE STRING "Dolibarr API Key")

# Chemins vers vos bibliothèques
set(M5STACK_PATH "${CMAKE_SOURCE_DIR}/lib/M5Stack_lib")
set(GRBL_PATH "${CMAKE_SOURCE_DIR}/lib/Module_GRBL_13.2")
set(MFRC522_PATH "${CMAKE_SOURCE_DIR}/lib/MFRC522_I2C")
set(ARDUINOJSON_PATH "${CMAKE_SOURCE_DIR}/lib/ArduinoJson")

# Configuration des sous-répertoires
add_subdirectory(src)
if(BUILD_TESTS AND NOT BUILD_FIRMWARE)
    add_subdirectory(test)
endif()