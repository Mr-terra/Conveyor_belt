#include "error_management.h"
#include "conveyor.h"
#include "sorter.h"
#include <Arduino.h>

// External objects
extern Conveyor conveyor;
extern Sorter sorter;

// Global variable for error detection
bool errorDetectedFlag = false;
String errorMessage = "";

void ErrorManagement::initialize() {
    errorDetectedFlag = false;
    Serial.println("ErrorManagement initialized");
}

void ErrorManagement::triggerError(String message) {
    errorMessage = message;
    errorDetectedFlag = true;
    Serial.print("Error detected: ");
    Serial.println(message);
}

bool ErrorManagement::isErrorDetected() {
    return errorDetectedFlag;
}

bool ErrorManagement::classifyError() {
    // Error classification
    Serial.println("Classifying error...");
    if (errorMessage == "Connection error") {
        return true; // Critical error
    } else if (errorMessage == "Initialization error") {
        return true; // Critical error
    } else {
        return false; // Non-critical error
    }
}

void ErrorManagement::autoRecovery() {
    // Auto-recovery
    Serial.println("Auto-recovery in progress...");
    errorDetectedFlag = false;
}

void ErrorManagement::safetyMeasures() {
    // Safety measures
    conveyor.stop();
    sorter.stop();
    Serial.println("Safety measures activated");
    errorDetectedFlag = false;
}