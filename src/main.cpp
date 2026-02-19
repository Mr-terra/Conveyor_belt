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
bool isStopButtonPressed = false;

void setup()
{

    // Initialize serial communication
    Serial.begin(115200);
    M5.begin();

    // I2C M5Stack (Core / grey) : SDA=21, SCL=22
    Wire.begin(21, 22);
    Wire.setClock(400000);

    for (uint8_t addr = 1; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("I2C device found at 0x");
            Serial.println(addr, HEX);
        }
    }

    // Initialiser le système
    conveyor.initialize(stateManager);

    Serial.println("🚀 System ready - Hardware monitoring active");
    Serial.println("BtnA: Start | BtnB: Stop | BtnC: Info");

    // Initialize all modules with automatic state monitoring
    scheduler.initialize();

    // Configuration OBLIGATOIRE des dépendances pour CheckConnect
    // Doit être fait avant tout appel à checkConnections()
    checkConnect.setDolibarrAPI(&dolibarrApi);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);

    // Configuration des dépendances pour le Scheduler
    scheduler.setDependencies(
        &checkConnect,
        &initPosition,
        &sorterController,
        &conveyor);

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
        // Bouton A: Démarrer le processus automatique
        String currentState = scheduler.getState();
        if (currentState == "WAITING")
        {
            scheduler.triggerCheckConnect();
        }
    }
    if (M5.BtnB.wasPressed())
    {
        // Bouton B: Arrêter et retourner à l'état initial
        scheduler.triggerStop();
    }
    if (M5.BtnC.wasPressed())
    {
        // Bouton C: Afficher l'état courant
        Serial.print("Current scheduler state: ");
        Serial.println(scheduler.getState());
        Serial.print("Conveyor state: ");
        Serial.println(conveyor.getState());
    }

    // Exécution de la logique de l'état courant via le scheduler
    // C'est le scheduler qui gère maintenant tout le flux de travail
    scheduler.executeCurrentState();

    delay(100);

    // Error management is now automatic through state monitoring
    // The StateManager will automatically execute safety measures for critical errors

    // Delay to avoid overload
    delay(100);
}