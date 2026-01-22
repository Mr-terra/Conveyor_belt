#include <Arduino.h>
#include "../include/state_error_manager.h"
#include "../include/conveyor.h"
#include "../include/sorter.h"
#include "../include/scheduler.h"
#include "../include/error.h"

// Mock external objects for testing
String conveyorState = CONVEYOR_STATE_STOPPED;
String sorterState = SORTER_STATE_INITIAL;
String schedulerState = STATE_WAITING;

StateErrorManager testStateErrorManager;

void setup() {
    Serial.begin(9600);
    while (!Serial); // Wait for serial connection
    
    Serial.println("Starting State and Error Management System Test...");
    
    // Setup state monitoring
    testStateErrorManager.observeState(&conveyorState, "Conveyor", Conveyor::stateToString);
    testStateErrorManager.observeState(&sorterState, "Sorter", Sorter::stateToString);
    testStateErrorManager.observeState(&schedulerState, "Scheduler", Scheduler::stateToString);
    
    // Add state transition rules
    testStateErrorManager.addStateRule("Conveyor", "STOPPED", "RUNNING", ERROR_INFO, "Conveyor started");
    testStateErrorManager.addStateRule("Conveyor", "RUNNING", "ERROR", ERROR_CRITICAL, "Conveyor failed");
    testStateErrorManager.addStateRule("Sorter", "INITIAL", "ERROR", ERROR_CRITICAL, "Sorter failed");
    testStateErrorManager.addStateRule("Scheduler", "WAITING", "PRODUCTION", ERROR_INFO, "Production started");
    
    Serial.println("Test setup complete.");
}

void loop() {
    Serial.println("\n=== Starting Test Sequence ===");
    
    // Test 1: Normal conveyor operation
    Serial.println("Test 1: Normal conveyor operation");
    conveyorState = CONVEYOR_STATE_STOPPED;
    testStateErrorManager.monitorAllStates();
    
    conveyorState = CONVEYOR_STATE_RUNNING;
    testStateErrorManager.monitorAllStates();
    
    // Test 2: Conveyor failure and recovery
    Serial.println("\nTest 2: Conveyor failure and recovery");
    conveyorState = CONVEYOR_STATE_ERROR;
    testStateErrorManager.monitorAllStates();
    
    // Test 3: Sorter failure
    Serial.println("\nTest 3: Sorter failure");
    sorterState = SORTER_STATE_ERROR;
    testStateErrorManager.monitorAllStates();
    
    // Test 4: Scheduler state change
    Serial.println("\nTest 4: Scheduler state change");
    schedulerState = STATE_PRODUCTION;
    testStateErrorManager.monitorAllStates();
    
    // Test 5: Manual error triggering
    Serial.println("\nTest 5: Manual error triggering");
    testStateErrorManager.triggerError("Manual test error", ERROR_WARNING, "TestModule");
    
    Serial.println("\n=== Test Sequence Complete ===");
    Serial.println("Waiting 10 seconds before repeating...");
    
    delay(10000); // Wait 10 seconds before repeating tests
}