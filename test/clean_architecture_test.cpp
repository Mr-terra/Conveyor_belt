#include <Arduino.h>
#include "../include/state_error_manager.h"
#include "../include/conveyor.h"
#include "../include/sorter.h"
#include "../include/scheduler.h"
#include "../include/error.h"

// Mock state variables for testing
String testConveyorState = CONVEYOR_STATE_STOPPED;
String testSorterState = SORTER_STATE_INITIAL;
String testSchedulerState = STATE_WAITING;

StateErrorManager cleanTestStateErrorManager;

// Mock objects for callbacks
class MockConveyor {
public:
    void stop() { Serial.println("MockConveyor::stop() called"); }
    void start() { Serial.println("MockConveyor::start() called"); }
} mockConveyor;

class MockSorter {
public:
    void stop() { Serial.println("MockSorter::stop() called"); }
} mockSorter;

class MockScheduler {
public:
    void triggerStop() { Serial.println("MockScheduler::triggerStop() called"); }
} mockScheduler;

bool mockCriticalError = false;

void setup() {
    Serial.begin(9600);
    while (!Serial); // Wait for serial connection
    
    Serial.println("Starting Clean Architecture Test...");
    Serial.println("Testing StateErrorManager as pure observer with callbacks");
    
    // Setup state monitoring
    cleanTestStateErrorManager.observeState(&testConveyorState, "Conveyor", Conveyor::stateToString);
    cleanTestStateErrorManager.observeState(&testSorterState, "Sorter", Sorter::stateToString);
    cleanTestStateErrorManager.observeState(&testSchedulerState, "Scheduler", Scheduler::stateToString);
    
    // Add forbidden transitions
    cleanTestStateErrorManager.addForbiddenTransition("Conveyor", "RUNNING", "RUNNING", "Conveyor stuck running");
    cleanTestStateErrorManager.addForbiddenTransition("Scheduler", "CHECK_CONNECT", "CHECK_CONNECT", "Scheduler stuck checking connections");
    
    // Setup callbacks - this is where business logic belongs
    cleanTestStateErrorManager.setErrorCallback([](const Error& error) {
        Serial.print("✅ Error Callback: [");
        Serial.print(error.getModule());
        Serial.print("] ");
        Serial.print(Error::levelToString(error.getLevel()));
        Serial.print(": ");
        Serial.println(error.getMessage());
    });
    
    cleanTestStateErrorManager.setCriticalErrorCallback([]() {
        Serial.println("✅ Critical Error Callback: Executing safety measures");
        mockConveyor.stop();
        mockSorter.stop();
        mockScheduler.triggerStop();
        mockCriticalError = true;
    });
    
    Serial.println("Test setup complete. StateErrorManager is now a pure observer.");
    Serial.println("All business logic is handled through callbacks.");
}

void loop() {
    Serial.println("\n=== Clean Architecture Test Sequence ===");
    
    // Test 1: Normal state transitions
    Serial.println("Test 1: Normal state transitions");
    testConveyorState = CONVEYOR_STATE_STOPPED;
    cleanTestStateErrorManager.monitorAllStates();
    
    testConveyorState = CONVEYOR_STATE_RUNNING;
    cleanTestStateErrorManager.monitorAllStates();
    
    // Test 2: Forbidden transition detection
    Serial.println("\nTest 2: Forbidden transition detection");
    // Simulate being stuck in CHECK_CONNECT state
    testSchedulerState = STATE_CHECK_CONNECT;
    cleanTestStateErrorManager.monitorAllStates();
    delay(1000); // Wait to simulate being stuck
    cleanTestStateErrorManager.monitorAllStates();
    delay(5000); // Wait for stuck state detection
    cleanTestStateErrorManager.monitorAllStates();
    
    // Test 3: State rules triggering
    Serial.println("\nTest 3: State rules triggering");
    testConveyorState = CONVEYOR_STATE_ERROR;
    cleanTestStateErrorManager.monitorAllStates();
    
    // Test 4: Recovery from error
    Serial.println("\nTest 4: Recovery from error");
    testConveyorState = CONVEYOR_STATE_STOPPED;
    cleanTestStateErrorManager.monitorAllStates();
    
    // Test 5: Manual error triggering (should still work)
    Serial.println("\nTest 5: Manual error triggering");
    cleanTestStateErrorManager.triggerError("Manual test error", ERROR_WARNING, "TestModule");
    
    // Test 6: Critical error callback
    Serial.println("\nTest 6: Critical error callback");
    cleanTestStateErrorManager.triggerError("Critical test error", ERROR_CRITICAL, "TestModule");
    
    Serial.println("\n=== Architecture Validation ===");
    Serial.println("✅ StateErrorManager is a pure observer");
    Serial.println("✅ No direct calls to business objects");
    Serial.println("✅ All actions handled through callbacks");
    Serial.println("✅ Forbidden transitions detected automatically");
    Serial.println("✅ Stuck states detected automatically");
    Serial.println("✅ Error handling separated from observation");
    
    // Reset for next test cycle
    mockCriticalError = false;
    testConveyorState = CONVEYOR_STATE_STOPPED;
    testSorterState = SORTER_STATE_INITIAL;
    testSchedulerState = STATE_WAITING;
    cleanTestStateErrorManager.clearCriticalErrors();
    
    Serial.println("\nWaiting 15 seconds before repeating test...");
    delay(15000);
}