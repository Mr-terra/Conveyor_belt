#include "conveyor.h"
#include "state_manager.h"
#include "error.h"

Conveyor conveyor(1, 1000);
extern String conveyorState;

// États basés sur les états réels du module GRBL
const String CONVEYOR_STATE_STOPPED = "STOPPED";    // 'I' (Idle)
const String CONVEYOR_STATE_RUNNING = "RUNNING";    // Autre que 'I' ou 'A'
const String CONVEYOR_STATE_ALARM = "ALARM";        // 'A' (Alarm/Lock)

Conveyor::Conveyor(int movement, int speed, uint8_t i2cAddress)
    : _movement(movement), _speed(speed), _grbl(i2cAddress), _i2cAddress(i2cAddress), _isRunning(false)
{
    if (_speed > 1000) {
        _speed = 1000;
    }
}

String Conveyor::stateToString(String state) {
    if (state == CONVEYOR_STATE_STOPPED) return "STOPPED";
    if (state == CONVEYOR_STATE_RUNNING) return "RUNNING";
    if (state == CONVEYOR_STATE_ALARM) return "ALARM";
    return "UNKNOWN";
}

void Conveyor::setupStateRules(StateManager& stateManager) {
    // Règles basées sur les transitions hardware réelles
    stateManager.addStateRule("Conveyor", "STOPPED", "RUNNING", ERROR_INFO, "Motor started (detected from hardware)");
    stateManager.addStateRule("Conveyor", "RUNNING", "STOPPED", ERROR_INFO, "Motor stopped (detected from hardware)");
    stateManager.addStateRule("Conveyor", "RUNNING", "ALARM", ERROR_CRITICAL, "Hardware alarm while running");
    stateManager.addStateRule("Conveyor", "STOPPED", "ALARM", ERROR_CRITICAL, "Hardware alarm while stopped");
    stateManager.addStateRule("Conveyor", "*", "ALARM", ERROR_FATAL, "GRBL ALARM - immediate stop required");
    stateManager.addStateRule("Conveyor", "ALARM", "STOPPED", ERROR_WARNING, "Recovered from alarm - unlocked");
}

void Conveyor::configureStateMonitoring(StateManager& stateManager, String* stateVariable) {
    stateManager.observeState(stateVariable, "Conveyor", Conveyor::stateToString);
    setupStateRules(stateManager);
}

void Conveyor::initialize(StateManager& stateManager) {
    M5.begin();
    
    // Initialiser le module GRBL
    _grbl.Init(&Wire);
    _grbl.setMode("distance");
    
    // Nettoyer le buffer
    _grbl.readClean();
    
    // Déverrouiller si nécessaire
    if (_grbl.inLock()) {
        Serial.println("⚠️ Module locked, unlocking...");
        _grbl.unLock();
        delay(100);
    }
    
    // Arrêter le moteur au démarrage
    _grbl.setMotor(0, 0, 0, 0);
    _grbl.waitIdle();
    
    // Lire l'état initial depuis le hardware
    readHardwareState();
    
    Serial.println("✅ Conveyor initialized with Module GRBL 13.2");
    Serial.print("📊 Initial state: ");
    Serial.println(conveyorState);
    
    configureStateMonitoring(stateManager, &conveyorState);
}

// ✨ CŒUR DU SYSTÈME : Lecture de l'état depuis le hardware
String Conveyor::readHardwareState() {
    String newState;
    
    // Lire le statut du module (retourne une String commençant par le code état)
    String status = _grbl.readStatus();
    
    if (status.length() > 0) {
        char stateChar = status[0];
        
        // Mapper les codes GRBL aux états du conveyor
        switch (stateChar) {
            case 'I':  // Idle
                newState = CONVEYOR_STATE_STOPPED;
                break;
                
            case 'A':  // Alarm (locked)
                newState = CONVEYOR_STATE_ALARM;
                break;
                
            case 'R':  // Run
            case 'J':  // Jog
            case 'H':  // Hold
            default:   // Tout autre état = en mouvement ou occupé
                if (stateChar != 'I' && stateChar != 'A') {
                    newState = CONVEYOR_STATE_RUNNING;
                } else {
                    newState = CONVEYOR_STATE_STOPPED;
                }
                break;
        }
        
        // Debug: afficher le code brut
        if (newState != conveyorState) {
            Serial.print("📡 Raw status: '");
            Serial.print(stateChar);
            Serial.print("' (");
            Serial.print((int)stateChar);
            Serial.println(")");
        }
    } else {
        // Pas de réponse, utiliser les fonctions helper
        if (_grbl.inLock()) {
            newState = CONVEYOR_STATE_ALARM;
        } else if (_grbl.readIdle()) {
            newState = CONVEYOR_STATE_STOPPED;
        } else {
            newState = CONVEYOR_STATE_RUNNING;
        }
    }
    
    // === MISE À JOUR DE L'ÉTAT (automatique) ===
    if (newState != conveyorState) {
        Serial.print("🔄 Hardware state change: ");
        Serial.print(conveyorState);
        Serial.print(" → ");
        Serial.println(newState);
        
        conveyorState = newState; // ← Seul endroit où on change l'état
        // Le StateManager détectera ce changement automatiquement
    }
    
    return newState;
}

// // ✨ SURVEILLANCE CONTINUE (appelée dans loop)
// void Conveyor::monitorHardware() {
//     static unsigned long lastCheck = 0;
//     unsigned long now = millis();
    
//     // Vérifier toutes les 100ms
//     if (now - lastCheck >= 100) {
//         // Lire l'état depuis le hardware
//         String currentState = readHardwareState();
        
//         // === DÉTECTIONS SUPPLÉMENTAIRES ===
        
//         // 1. Détection d'alarme inattendue
//         if (currentState == CONVEYOR_STATE_ALARM) {
//             static bool alarmLogged = false;
//             if (!alarmLogged) {
//                 Error::triggerError("GRBL alarm detected - module locked", ERROR_FATAL, "Conveyor");
//                 alarmLogged = true;
//             }
//         } else {
//             static bool alarmLogged = false;
//             alarmLogged = false;  // Reset
//         }
        
//         // 2. Détection de moteur qui devrait tourner mais est idle
//         if (_isRunning && currentState == CONVEYOR_STATE_STOPPED) {
//             static unsigned long idleTime = 0;
//             if (idleTime == 0) {
//                 idleTime = now;
//             } else if (now - idleTime > 1000) {
//                 // Le moteur devrait tourner mais est idle depuis 1 seconde
//                 Error::triggerError("Motor expected to run but is idle", ERROR_WARNING, "Conveyor");
//                 _isRunning = false;  // Corriger l'état interne
//                 idleTime = 0;
//             }
//         }
        
//         lastCheck = now;
//     }
// }

// ✨ SURVEILLANCE SIMPLIFIÉE - AUCUN triggerError manuel
void Conveyor::monitorHardware() {
    static unsigned long lastCheck = 0;
    unsigned long now = millis();
    
    if (now - lastCheck >= 100) {
        // Lire l'état depuis le hardware
        readHardwareState();
        // ↑ C'est TOUT ! Les rules du StateManager gèrent le reste
        
        // Réinitialiser le flag _isRunning si le moteur est arrêté
        if (conveyorState == CONVEYOR_STATE_STOPPED) {
            _isRunning = false;
        }
        
        lastCheck = now;
    }
}

// ✨ COMMANDES SIMPLIFIÉES (pas de setState manuel)
void Conveyor::start() {
    // Déverrouiller si nécessaire
    if (_grbl.inLock()) {
        Serial.println("🔓 Unlocking module...");
        _grbl.unLock();
        delay(100);
    }
    
    _isRunning = true;
    Serial.println("✅ Conveyor start command sent");
    // L'état passera à RUNNING automatiquement via readHardwareState()
}

void Conveyor::stop() {
    _grbl.setMotor(0, 0, 0, 0);
    _grbl.waitIdle();  // Attendre que le moteur soit vraiment arrêté
    _isRunning = false;
    Serial.println("🛑 Conveyor stop command sent");
    // L'état passera à STOPPED automatiquement via readHardwareState()
}

void Conveyor::move(bool forward) {
    if (!_isRunning) {
        Serial.println("⚠️ Conveyor not running, call start() first");
        return;
    }
    
    int angle = forward ? _movement : -_movement;
    _grbl.setMotor(angle, 0, 0, _speed);
    // L'état sera mis à jour automatiquement par monitorHardware()
}

void Conveyor::setSpeed(int speed) {
    _speed = constrain(speed, 0, 1000);
}

void Conveyor::setMovement(int steps) {
    _movement = steps;
}

String Conveyor::getState() const {
    return conveyorState;
}

void Conveyor::setState(String state) {
    Serial.println("⚠️ WARNING: setState() is deprecated - state is read from hardware");
    Serial.println("⚠️ Use readHardwareState() instead");
}