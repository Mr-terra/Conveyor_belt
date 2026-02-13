#include <Arduino.h>
#include "conveyor.h"
#include "scheduler.h"
#include "check_connect.h"
#include "init_position.h"
#include "state_manager.h"
#include "main_sorter.h"
#include "error.h"
#include "dolibarr_api.h"

// Global objects
extern Conveyor conveyor;
extern Scheduler scheduler;
extern CheckConnect checkConnect;
extern InitPosition initPosition;
extern StateManager stateManager;
extern SorterController sorterController;

// Configuration Dolibarr (à adapter selon votre environnement)
// Remplacez par l'URL et la clé API de votre instance Dolibarr
const String DOLIBARR_URL = "http://votre-serveur-dolibarr.com/api/index.php";
const String DOLIBARR_API_KEY = "votre-cle-api-dolibarr";

DolibarrAPI dolibarrApi(DOLIBARR_URL, DOLIBARR_API_KEY);

// Global variables for states and conditions
bool isConnectionOk = false;
bool isProductionOk = false;
bool isStopButtonPressed = false;
bool isCriticalError = false;

// Timing pour mouvement continu
unsigned long lastMoveTime = 0;
const unsigned long moveInterval = 135; // ms entre chaque mouvement

void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    M5.begin();

    // Initialiser le système
    conveyor.initialize(stateManager);
    sorterController.initialize(stateManager);

    Serial.println("🚀 System ready - Hardware monitoring active");
    Serial.println("BtnA: Start | BtnB: Stop | BtnC: Info");

    // Initialize all modules with automatic state monitoring
    // conveyor.initialize(stateManager);
    scheduler.initialize();
    
    // Configuration OBLIGATOIRE des dépendances pour CheckConnect
    // Doit être fait avant tout appel à checkConnections()
    checkConnect.setDolibarrAPI(&dolibarrApi);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);
    initPosition.initialize();

    // Setup automatic state monitoring and callbacks
    // setupStateMonitoring();

    Serial.println("System initialized. State: WAITING");
}

void loop()
{
    M5.update();

    // ✨ ÉTAPE 1 : Lire l'état depuis le hardware
    conveyor.monitorHardware();

    // ✨ ÉTAPE 2 : Le StateManager détecte les changements et applique les rules
    stateManager.monitorAllStates();

    // Contrôle utilisateur
    if (M5.BtnA.wasPressed())
    {
        conveyor.start();
    }
    if (M5.BtnB.wasPressed())
    {
        conveyor.stop();
    }
    if (M5.BtnC.wasPressed())
    {
        Serial.print("Current state (from hardware): ");
        Serial.println(conveyor.getState());
    }

    // Mouvement continu si en marche
    if (conveyor.getState() == CONVEYOR_STATE_RUNNING)
    {
        if (millis() - lastMoveTime >= moveInterval)
        {
            conveyor.move(true);
            lastMoveTime = millis();
        }
    }

    delay(10);

    // Automatic state monitoring - detects all state changes and applies rules
    stateManager.monitorAllStates();

    // State management by the scheduler
    String currentState = scheduler.getState();

    if (currentState == "WAITING")
    {
        // Waiting for trigger
        if (!isStopButtonPressed)
        {
            scheduler.triggerCheckConnect();
        }
    }
    else if (currentState == "CHECK_CONNECT")
    {
        // ✨ Pattern standalone : toutes les méthodes sont static
        isConnectionOk = checkConnect.checkConnections();
        
        if (isConnectionOk)
        {
            scheduler.triggerInitPositions();
        }
        else
        {
            // Gestion d'erreur : retour à l'état WAITING
            Serial.println("❌ Connection check failed - returning to WAITING");
            scheduler.setState("WAITING");
        }
    }
    else if (currentState == "INIT_POSITIONS")
    {
        // Initialize positions
        isProductionOk = initPosition.initializePositions();
        if (isProductionOk)
        {
            scheduler.triggerProduction();
        }
        else
        {
            // Error will be detected automatically through state monitoring
            // The system will detect that we're stuck in INIT_POSITIONS state
        }
    }
    else if (currentState == "PRODUCTION")
    {
        conveyor.move();
        sorterController.production();
    }

    // Error management is now automatic through state monitoring
    // The StateManager will automatically execute safety measures for critical errors

    // Delay to avoid overload
    delay(100);
}