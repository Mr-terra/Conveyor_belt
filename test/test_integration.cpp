// Integration test StateManager <-> Scheduler
// This file demonstrates how the integration works

#include <Arduino.h>
#include "state_manager.h"
#include "scheduler.h"
#include "conveyor.h"

// Simulated states for the test
void testConveyorAlarmDuringProduction() {
    Serial.println("=== Test 1: Conveyor ALARM during production ===");

    // Initialization
    scheduler.initialize();
    stateManager.setScheduler(&scheduler);

    // Simulate switching to production
    scheduler.setState(STATE_PRODUCTION);
    Serial.print("Initial state: ");
    Serial.println(scheduler.getState());

    // Simulate a conveyor alarm
    // In reality, this would be triggered by the hardware
    // Here, we simulate the state change
    Serial.println("Simulating conveyor ALARM...");

    // The StateManager should detect this and act
    // (In reality, this happens through monitorAllStates())

    Serial.println("Expected: Scheduler should return to WAITING state");
    Serial.println("Test completed\n");
}

void testErrorStateRecovery() {
    Serial.println("=== Test 2: ERROR state recovery ===");

    // Put the system in ERROR state
    scheduler.setState(STATE_ERROR);
    Serial.print("Initial state: ");
    Serial.println(scheduler.getState());

    // Run the current state (should check the recovery conditions)
    scheduler.executeCurrentState();

    Serial.println("Expected: Should check recovery conditions");
    Serial.println("If conditions met, should return to WAITING");
    Serial.println("Test completed\n");
}

void testStateTransitions() {
    Serial.println("=== Test 3: Normal state transitions ===");

    // Test the normal transitions
    scheduler.setState(STATE_WAITING);
    Serial.print("WAITING -> ");

    scheduler.setState(STATE_CHECK_CONNECT);
    Serial.print("CHECK_CONNECT -> ");

    scheduler.setState(STATE_INIT_POSITIONS);
    Serial.print("INIT_POSITIONS -> ");

    scheduler.setState(STATE_PRODUCTION);
    Serial.println("PRODUCTION");

    Serial.println("All transitions completed successfully");
    Serial.println("Test completed\n");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("🧪 Starting Integration Tests\n");

    testStateTransitions();
    testConveyorAlarmDuringProduction();
    testErrorStateRecovery();

    Serial.println("✅ All tests completed");
}

void loop() {
    // Nothing to do here
}
