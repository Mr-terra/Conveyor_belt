#include "conveyor.h"
#include "state_manager.h"
#include "error.h"
#include <Arduino.h>

Conveyor conveyor;
// External state variable (defined in main.cpp)
extern String conveyorState;

// Constants for conveyor motor pin
#define CONVEYOR_MOTOR_PIN 5

// State constants for conveyor (now declared in header)
const String CONVEYOR_STATE_STOPPED = "STOPPED";
const String CONVEYOR_STATE_RUNNING = "RUNNING";
const String CONVEYOR_STATE_ERROR = "ERROR";

// State conversion function for automatic monitoring
String Conveyor::stateToString(String state) {
    if (state == CONVEYOR_STATE_STOPPED) return "STOPPED";
    if (state == CONVEYOR_STATE_RUNNING) return "RUNNING";
    if (state == CONVEYOR_STATE_ERROR) return "ERROR";
    return "UNKNOWN";
}

// Module self-configuration - setup state transition rules
void Conveyor::setupStateRules(StateManager& stateManager) {
    // Define conveyor-specific state transition rules
    stateManager.addStateRule("Conveyor", "STOPPED", "RUNNING", ERROR_INFO, "Conveyor started successfully");
    stateManager.addStateRule("Conveyor", "RUNNING", "STOPPED", ERROR_WARNING, "Conveyor stopped normally");
    stateManager.addStateRule("Conveyor", "RUNNING", "ERROR", ERROR_CRITICAL, "Conveyor entered error state from running");
    stateManager.addStateRule("Conveyor", "STOPPED", "ERROR", ERROR_CRITICAL, "Conveyor entered error state from stopped");
    stateManager.addStateRule("Conveyor", "ERROR", "STOPPED", ERROR_INFO, "Conveyor recovered from error");
    stateManager.addStateRule("Conveyor", "ERROR", "RUNNING", ERROR_WARNING, "Conveyor recovered and started");
}

// Module state monitoring configuration
void Conveyor::configureStateMonitoring(StateManager& stateManager, String* stateVariable) {
    // Configure state monitoring for this conveyor instance
    stateManager.observeState(stateVariable, "Conveyor", Conveyor::stateToString);
    
    // Setup the state transition rules
    setupStateRules(stateManager);
}

void Conveyor::initialize(StateManager& stateManager) {
    conveyorState = CONVEYOR_STATE_STOPPED;
    digitalWrite(CONVEYOR_MOTOR_PIN, LOW);

    // Basic initialization without state monitoring
    Serial.println("Conveyor initialized");

    configureStateMonitoring(stateManager, &conveyorState);
    
    Serial.println("Conveyor initialized with automatic state monitoring");
}

void Conveyor::start() {
    bool isMotorRunning = true; // Simulate motor start
    if (!isMotorRunning) {
        // Let the state monitoring system detect this error automatically
        // through the checkState() method and state transition rules
        setState(CONVEYOR_STATE_ERROR);
        return;
    }
    setState(CONVEYOR_STATE_RUNNING);
}

void Conveyor::stop() {
    setState(CONVEYOR_STATE_STOPPED);
}

String Conveyor::getState() const {
    return conveyorState;
}

void Conveyor::setState(String state) {
    String previousState = conveyorState;
    conveyorState = state;

    // Update motor state
    if (state == CONVEYOR_STATE_RUNNING) {
        digitalWrite(CONVEYOR_MOTOR_PIN, HIGH);
    } else {
        digitalWrite(CONVEYOR_MOTOR_PIN, LOW);
    }
    
    // State changes will be detected automatically by StateManager
    // through the monitorAllStates() call in the main loop
}




