#include "conveyor.h"
#include "state_error_manager.h"
#include <Arduino.h>

// External state variable (defined in main.cpp)
extern String conveyorState;

// Constants for conveyor motor pin
#define CONVEYOR_MOTOR_PIN 5

// State constants for conveyor (now declared in header)
const String CONVEYOR_STATE_STOPPED = "STOPPED";
const String CONVEYOR_STATE_RUNNING = "RUNNING";
const String CONVEYOR_STATE_ERROR = "ERROR";

// State conversion function for automatic monitoring
// String Conveyor::stateToString(String state) {
//     if (state == CONVEYOR_STATE_STOPPED) return "STOPPED";
//     if (state == CONVEYOR_STATE_RUNNING) return "RUNNING";
//     if (state == CONVEYOR_STATE_ERROR) return "ERROR";
//     return "UNKNOWN";
// }

// Module self-configuration - setup state transition rules
void Conveyor::setupStateRules(StateErrorManager& stateErrorManager) {
    // Define conveyor-specific state transition rules
    stateErrorManager.addStateRule("Conveyor", "STOPPED", "RUNNING", ERROR_INFO, "Conveyor started successfully");
    stateErrorManager.addStateRule("Conveyor", "RUNNING", "STOPPED", ERROR_WARNING, "Conveyor stopped normally");
    stateErrorManager.addStateRule("Conveyor", "RUNNING", "ERROR", ERROR_CRITICAL, "Conveyor entered error state from running");
    stateErrorManager.addStateRule("Conveyor", "STOPPED", "ERROR", ERROR_CRITICAL, "Conveyor entered error state from stopped");
    stateErrorManager.addStateRule("Conveyor", "ERROR", "STOPPED", ERROR_INFO, "Conveyor recovered from error");
    stateErrorManager.addStateRule("Conveyor", "ERROR", "RUNNING", ERROR_WARNING, "Conveyor recovered and started");
    
    // Add forbidden transitions
    stateErrorManager.addForbiddenTransition("Conveyor", "ERROR", "ERROR", "Conveyor stuck in error state");
}

// Module state monitoring configuration
void Conveyor::configureStateMonitoring(StateErrorManager& stateErrorManager, String* stateVariable) {
    // Configure state monitoring for this conveyor instance
    stateErrorManager.observeState(stateVariable, "Conveyor", Conveyor::stateToString);
    
    // Setup the state transition rules
    setupStateRules(stateErrorManager);
    
    // Setup physical state checks for automatic error detection
    setupPhysicalChecks(stateErrorManager);
}

// Setup physical state checks for automatic error detection
void Conveyor::setupPhysicalChecks(StateErrorManager& stateErrorManager) {
    // Check that motor is running when in RUNNING state
    stateErrorManager.addPhysicalCheck("Conveyor", "RUNNING", []() {
        return digitalRead(CONVEYOR_MOTOR_PIN) == HIGH;
    }, "Conveyor motor not running while in RUNNING state", ERROR_CRITICAL);
    
    // Additional physical checks could be added here
    // For example: overheating, belt alignment, etc.
}

void Conveyor::initialize() {
    conveyorState = CONVEYOR_STATE_STOPPED;
    digitalWrite(CONVEYOR_MOTOR_PIN, LOW);

    // Basic initialization without state monitoring
    Serial.println("Conveyor initialized (basic)");
}

void Conveyor::initializeWithStateMonitoring(StateErrorManager& stateErrorManager) {
    // Initialize the conveyor
    initialize();
    
    // Configure automatic state monitoring
    configureStateMonitoring(stateErrorManager, &conveyorState);
    
    Serial.println("Conveyor initialized with automatic state monitoring");
}

void Conveyor::start() {
    if (!isMotorRunning()) {
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
    
    // State changes will be detected automatically by StateErrorManager
    // through the monitorAllStates() call in the main loop
}




