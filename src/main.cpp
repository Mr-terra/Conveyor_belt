#include <Arduino.h>
#include "conveyor.h"
#include "scheduler.h"
#include "check_connect.h"
#include "init_position.h"
#include "state_manager.h"
#include "main_sorter.h"
#include "error.h"
#include "dolibarr_api.h"
#include "adresser.h"

// Global objects
extern Conveyor conveyor;
extern Scheduler scheduler;
extern CheckConnect checkConnect;
extern InitPosition initPosition;
extern StateManager stateManager;
extern SorterController sorterController;

Adresser adresser;

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
    Wire.begin(21, 22); // M5Stack Core = SDA 21 / SCL 22
    Wire.setClock(400000);
    M5.begin();

    // Initialiser le système
    conveyor.initialize(stateManager);
    sorterController.initialize(stateManager);

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

    // Démarrer le convoyeur UNE SEULE FOIS
    // conveyor.start();

    // Affichage initial
    M5.Lcd.clear();
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Conveyor Control");
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.println("A: Forward  C: Backward");
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.println("B: Toggle Mode");
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.print("Mode: ");
    M5.Lcd.println(scheduler.getState());

    // Démarrer en mode WAITING
    scheduler.triggerStop();

    // instancie Dolibard pour l'adresser
    adresser.setDolibarrAPI(&dolibarrApi);

    Serial.println("System initialized. State: WAITING");
}

void updateDisplay()
{
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.print("Mode: ");
    M5.Lcd.println(scheduler.getState());

    M5.Lcd.setCursor(10, 130);
    M5.Lcd.print("State: ");
    M5.Lcd.println(conveyor.getState());
}

void loop()
{
    M5.update();

    // ✨ ÉTAPE 1 : Lire l'état depuis le hardware
    conveyor.monitorHardware();

    // ✨ ÉTAPE 2 : Le StateManager détecte les changements et applique les rules
    stateManager.monitorAllStates();

    // Gestion des boutons
    if (M5.BtnA.wasPressed())
    {
        Serial.println("🔄 Bouton A pressed - Forward");
        conveyor.move(true);
        updateDisplay();
    }
    else if (M5.BtnC.wasPressed())
    {
        Serial.println("🔄 Bouton C pressed - Backward");
        conveyor.move(false);
        updateDisplay();
    }
    else if (M5.BtnB.wasPressed())
    {
        Serial.println("🔄 Bouton B pressed - Toggle Mode");
        if (scheduler.getState() == "WAITING")
        {
            scheduler.triggerProduction();
        }
        else
        {
            scheduler.triggerStop();
        }
        updateDisplay();
    }
    // Exécution de la logique de l'état courant via le scheduler
    // C'est le scheduler qui gère maintenant tout le flux de travail
    scheduler.executeCurrentState();
    // Error management is now automatic through state monitoring
    // The StateManager will automatically execute safety measures for critical errors

    // Delay to avoid overload
    delay(100);
}