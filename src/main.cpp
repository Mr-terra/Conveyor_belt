#include <Arduino.h>
#include "conveyor.h"
#include "scheduler.h"
#include "check_connect.h"
#include "init_position.h"
#include "production.h"
#include "state_error_manager.h"
#include "sorter.h"
#include "package_sorter.h"
#include "error.h"

// Constants for stop button pin
#define STOP_BUTTON_PIN 8

// Global objects
Conveyor conveyor;
Scheduler scheduler;
CheckConnect checkConnect;
InitPosition initPosition;
Production production;
StateErrorManager stateErrorManager;
Sorter sorter;
PackageSorter packageSorter;

// Global variables for states and conditions
bool isConnectionOk = false;
bool isProductionOk = false;
bool isStopButtonPressed = false;
bool isCriticalError = false;

// Global state variables for automatic monitoring
String conveyorState = CONVEYOR_STATE_STOPPED;
String sorterState = SORTER_STATE_INITIAL;
String schedulerState = STATE_WAITING;

void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    Serial.println("System initialization...");

    // Initialize stop button pin
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

    // Initialize all modules with automatic state monitoring
    conveyor.initializeWithStateMonitoring(stateErrorManager);
    scheduler.initialize();
    checkConnect.initialize();
    initPosition.initialize();
    production.initialize();
    sorter.initialize();
    packageSorter.initialize();

    // Setup automatic state monitoring and callbacks
    setupStateMonitoring();

    Serial.println("System initialized. State: WAITING");
}

void setupStateMonitoring() {
    // Start automatic monitoring
    stateErrorManager.startAutomaticMonitoring();

    // Configure modules that don't auto-configure themselves yet
    stateErrorManager.observeState(&sorterState, "Sorter", Sorter::stateToString);
    stateErrorManager.observeState(&schedulerState, "Scheduler", Scheduler::stateToString);

    // Add system-level forbidden transitions
    stateErrorManager.addForbiddenTransition("Scheduler", "CHECK_CONNECT", "CHECK_CONNECT", 
        "Stuck in CHECK_CONNECT state - connection failed");
    stateErrorManager.addForbiddenTransition("Scheduler", "INIT_POSITIONS", "INIT_POSITIONS", 
        "Stuck in INIT_POSITIONS state - initialization failed");
    stateErrorManager.addForbiddenTransition("Sorter", "ERROR", "ERROR", 
        "Sorter stuck in error state");

    // Setup callbacks for error handling - separation of concerns
    stateErrorManager.setErrorCallback([](const Error& error) {
        Serial.print("⚠️  Error Callback: [");
        Serial.print(error.getModule());
        Serial.print("] ");
        Serial.print(error.toString());
        Serial.print(": ");
        Serial.println(error.getMessage());
        
        // Handle different error levels appropriately
        if (error.getLevel() == ERROR_WARNING) {
            Serial.println("   → WARNING level - logging only");
        }
    });

    stateErrorManager.setCriticalErrorCallback([]() {
        Serial.println("🚨 CRITICAL ERROR Callback: Executing safety measures");
        conveyor.stop();
        sorter.stop();
        scheduler.triggerStop();
        isCriticalError = true;
    });
}

void loop() {
    // Read stop button
    isStopButtonPressed = digitalRead(STOP_BUTTON_PIN) == LOW;

    // Update global state variables for automatic monitoring
    conveyorState = conveyor.getState();
    sorterState = sorter.getState();
    schedulerState = scheduler.getState();

    // Automatic state monitoring - detects all state changes and applies rules
    stateErrorManager.monitorAllStates();

    // State management by the scheduler
    String currentState = scheduler.getState();

    if (currentState == "WAITING") {
        // Waiting for trigger
        if (!isStopButtonPressed) {
            scheduler.triggerCheckConnect();
        }
    } else if (currentState == "CHECK_CONNECT") {
        // Check connections
        isConnectionOk = checkConnect.checkConnections();
        if (isConnectionOk) {
            scheduler.triggerInitPositions();
        } else {
            // Error will be detected automatically through state monitoring
            // The system will detect that we're stuck in CHECK_CONNECT state
        }
    } else if (currentState == "INIT_POSITIONS") {
        // Initialize positions
        isProductionOk = initPosition.initializePositions();
        if (isProductionOk) {
            scheduler.triggerProduction();
        } else {
            // Error will be detected automatically through state monitoring
            // The system will detect that we're stuck in INIT_POSITIONS state
        }
    } else if (currentState == "PRODUCTION") {
        // Production cycle
        if (isStopButtonPressed || isCriticalError) {
            production.stopProduction();
            scheduler.triggerStop();
        } else {
            production.startProduction();
            // Read packages and sort
            String packageInfo = production.readPackage();
            if (!packageInfo.isEmpty()) {
                packageSorter.sortPackage(packageInfo);
            }
        }
    }

    // Error management is now automatic through state monitoring
    // The StateErrorManager will automatically execute safety measures for critical errors

    // Delay to avoid overload
    delay(100);
}