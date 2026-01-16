#include <Arduino.h>
#include "conveyor.h"
#include "scheduler.h"
#include "check_connect.h"
#include "init_position.h"
#include "production.h"
#include "error_management.h"
#include "sorter.h"
#include "package_sorter.h"

// Constants for stop button pin
#define STOP_BUTTON_PIN 8

// Global objects
Conveyor conveyor;
Scheduler scheduler;
CheckConnect checkConnect;
InitPosition initPosition;
Production production;
ErrorManagement errorManagement;
Sorter sorter;
PackageSorter packageSorter;

// Global variables for states and conditions
bool isConnectionOk = false;
bool isProductionOk = false;
bool isStopButtonPressed = false;
bool isCriticalError = false;

void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    Serial.println("System initialization...");

    // Initialize stop button pin
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

    // Initialize all modules
    conveyor.initialize();
    scheduler.initialize();
    checkConnect.initialize();
    initPosition.initialize();
    production.initialize();
    errorManagement.initialize();
    sorter.initialize();
    packageSorter.initialize();

    Serial.println("System initialized. State: WAITING");
}

void loop() {
    // Read stop button
    isStopButtonPressed = digitalRead(STOP_BUTTON_PIN) == LOW;

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
            errorManagement.triggerError("Connection error");
        }
    } else if (currentState == "INIT_POSITIONS") {
        // Initialize positions
        isProductionOk = initPosition.initializePositions();
        if (isProductionOk) {
            scheduler.triggerProduction();
        } else {
            errorManagement.triggerError("Initialization error");
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

    // Error management
    if (errorManagement.isErrorDetected()) {
        isCriticalError = errorManagement.classifyError();
        if (isCriticalError) {
            errorManagement.safetyMeasures();
            scheduler.triggerStop();
        } else {
            errorManagement.autoRecovery();
        }
    }

    // Delay to avoid overload
    delay(100);
}