#include "production.h"
#include "conveyor.h"
#include <Arduino.h>

Production production;

// External conveyor object
extern Conveyor conveyor;

void Production::initialize() {
    Serial.println("Production initialized");
}

void Production::startProduction() {
    conveyor.start();
    Serial.println("Production started");
}

void Production::stopProduction() {
    conveyor.stop();
    Serial.println("Production stopped");
}

String Production::readPackage() {
    // Simulation of reading package
    Serial.println("Reading package...");
    return "Package_001";
}