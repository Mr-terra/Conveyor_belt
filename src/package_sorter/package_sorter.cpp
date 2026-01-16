#include "package_sorter.h"
#include "sorter.h"
#include <Arduino.h>

// External sorter object
extern Sorter sorter;

void PackageSorter::initialize() {
    Serial.println("PackageSorter initialized");
}

String PackageSorter::defineDirection(String packageInfo) {
    // Simulation of direction definition
    Serial.print("Defining direction for ");
    Serial.println(packageInfo);
    // Here you would add the actual logic to define the direction based on package info
    return "A";
}

void PackageSorter::sortPackage(String packageInfo) {
    String direction = defineDirection(packageInfo);
    sorter.moveTo(direction);
}