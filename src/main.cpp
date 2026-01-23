#include <Arduino.h>
#include "conveyor.h"
#include "scheduler.h"
#include "check_connect.h"
#include "init_position.h"
#include "production.h"
#include "state_manager.h"
#include "sorter.h"
#include "package_sorter.h"
#include "error.h"

// Constants for stop button pin
// #define STOP_BUTTON_PIN 8

// Global objects
extern Conveyor conveyor;
extern Scheduler scheduler;
extern CheckConnect checkConnect;
extern InitPosition initPosition;
extern Production production;
extern StateManager stateManager;
extern Sorter sorter;
extern PackageSorter packageSorter;

// Global variables for states and conditions
bool isConnectionOk = false;
bool isProductionOk = false;
bool isStopButtonPressed = false;
bool isCriticalError = false;

// Global state variables for automatic monitoring
String conveyorState = CONVEYOR_STATE_STOPPED;
String sorterState = SORTER_STATE_INITIAL;
String schedulerState = STATE_WAITING;

// Timing pour mouvement continu
unsigned long lastMoveTime = 0;
const unsigned long moveInterval = 135; // ms entre chaque mouvement

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    M5.begin();
    
    // Initialiser le système
    conveyor.initialize(stateManager);
    
    Serial.println("🚀 System ready - Hardware monitoring active");
    Serial.println("BtnA: Start | BtnB: Stop | BtnC: Info");

    // Initialize all modules with automatic state monitoring
    // conveyor.initialize(stateManager);
    scheduler.initialize();
    checkConnect.initialize();
    initPosition.initialize();
    production.initialize();
    sorter.initialize();
    packageSorter.initialize();

    // Setup automatic state monitoring and callbacks
    // setupStateMonitoring();

    Serial.println("System initialized. State: WAITING");
}

// void setupStateMonitoring() {
//     // Start automatic monitoring
//     stateErrorManager.startAutomaticMonitoring();

//     // Configure modules that don't auto-configure themselves yet
//     stateErrorManager.observeState(&sorterState, "Sorter", Sorter::stateToString);
//     stateErrorManager.observeState(&schedulerState, "Scheduler", Scheduler::stateToString);

//     // Add system-level forbidden transitions
//     stateErrorManager.addForbiddenTransition("Scheduler", "CHECK_CONNECT", "CHECK_CONNECT", 
//         "Stuck in CHECK_CONNECT state - connection failed");
//     stateErrorManager.addForbiddenTransition("Scheduler", "INIT_POSITIONS", "INIT_POSITIONS", 
//         "Stuck in INIT_POSITIONS state - initialization failed");
//     stateErrorManager.addForbiddenTransition("Sorter", "ERROR", "ERROR", 
//         "Sorter stuck in error state");

//     // Setup callbacks for error handling - separation of concerns
//     stateErrorManager.setErrorCallback([](const Error& error) {
//         Serial.print("⚠️  Error Callback: [");
//         Serial.print(error.getModule());
//         Serial.print("] ");
//         Serial.print(error.toString());
//         Serial.print(": ");
//         Serial.println(error.getMessage());
        
//         // Handle different error levels appropriately
//         if (error.getLevel() == ERROR_WARNING) {
//             Serial.println("   → WARNING level - logging only");
//         }
//     });

//     stateErrorManager.setCriticalErrorCallback([]() {
//         Serial.println("🚨 CRITICAL ERROR Callback: Executing safety measures");
//         conveyor.stop();
//         sorter.stop();
//         scheduler.triggerStop();
//         isCriticalError = true;
//     });
// }

void loop() {
    M5.update();
    
    // ✨ ÉTAPE 1 : Lire l'état depuis le hardware
    conveyor.monitorHardware();
    
    // ✨ ÉTAPE 2 : Le StateManager détecte les changements et applique les rules
    stateManager.monitorAllStates();
    
    // Contrôle utilisateur
    if (M5.BtnA.wasPressed()) {
        conveyor.start();
    }
    if (M5.BtnB.wasPressed()) {
        conveyor.stop();
    }
    if (M5.BtnC.wasPressed()) {
        Serial.print("Current state (from hardware): ");
        Serial.println(conveyorState);
    }
    
    // Mouvement continu si en marche
    if (conveyorState == CONVEYOR_STATE_RUNNING) {
        if (millis() - lastMoveTime >= moveInterval) {
            conveyor.move(true);
            lastMoveTime = millis();
        }
    }
    
    delay(10);

    // Update global state variables for automatic monitoring
    conveyorState = conveyor.getState();
    sorterState = sorter.getState();
    schedulerState = scheduler.getState();

    // Automatic state monitoring - detects all state changes and applies rules
    stateManager.monitorAllStates();

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
    // The StateManager will automatically execute safety measures for critical errors

    // Delay to avoid overload
    delay(100);
}