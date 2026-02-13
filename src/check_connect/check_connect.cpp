#include "check_connect.h"
#include "conveyor.h"
#include "main_sorter.h"
#include "dolibarr_api.h"
#include <Arduino.h>

CheckConnect checkConnect;
extern Conveyor conveyor;
extern SorterController sorterController;
extern DolibarrAPI dolibarrApi;

// Pointeurs vers les dépendances (injection de dépendances)
static Conveyor* _conveyor = nullptr;
static SorterController* _sorterController = nullptr;
static DolibarrAPI* _dolibarrApi = nullptr;

void CheckConnect::setDolibarrAPI(DolibarrAPI* api) {
    _dolibarrApi = api;
}

void CheckConnect::setConveyor(Conveyor* conv) {
    _conveyor = conv;
}

void CheckConnect::setSorterController(SorterController* sorter) {
    _sorterController = sorter;
}

// Méthode pour vérifier que toutes les dépendances sont configurées
bool CheckConnect::areDependenciesConfigured() {
    bool allConfigured = (_conveyor != nullptr) && 
                        (_sorterController != nullptr) && 
                        (_dolibarrApi != nullptr);
    
    if (!allConfigured) {
        Serial.println("⚠️  CheckConnect: Some dependencies not configured!");
        if (_conveyor == nullptr) Serial.println("  - Conveyor not set");
        if (_sorterController == nullptr) Serial.println("  - SorterController not set");
        if (_dolibarrApi == nullptr) Serial.println("  - DolibarrAPI not set");
    }
    
    return allConfigured;
}

bool CheckConnect::checkConnections() {
    Serial.println("\n🔍 === Checking all connections ===");
    
    // Vérifier que toutes les dépendances sont configurées
    if (!areDependenciesConfigured()) {
        Serial.println("❌ === Connection check ABORTED - dependencies not configured ===\n");
        return false;
    }
    
    // Check all connections using instance methods
    bool dolibarrOk = checkDolibarrConnection();
    bool motorOk = checkMotorConnection();
    bool sorterOk = checkSorterConnection();

    bool allOk = dolibarrOk && motorOk && sorterOk;
    
    if (allOk) {
        Serial.println("✅ === All connections OK ===\n");
    } else {
        Serial.println("❌ === Connection check FAILED ===\n");
    }
    
    return allOk;
}

bool CheckConnect::checkDolibarrConnection() {
    Serial.println("🌐 Checking Dolibarr connection...");
    
    if (_dolibarrApi == nullptr) {
        Serial.println("⚠️  Dolibarr API not configured (skipping check)");
        return true; // Assume OK if not configured
    }
    
    bool connected = _dolibarrApi->checkConnection();
    
    if (connected) {
        Serial.println("✅ Dolibarr connection OK");
        return true;
    } else {
        Serial.println("❌ Dolibarr connection FAILED");
        return false;
    }
}

bool CheckConnect::checkMotorConnection() {
    Serial.println("🔧 Checking Motor connection...");
    
    if (_conveyor == nullptr) {
        Serial.println("❌ Conveyor not initialized");
        return false;
    }
    
    return _conveyor->checkConnection();
}

bool CheckConnect::checkSorterConnection() {
    Serial.println("📦 Checking Sorter connection...");
    
    if (_sorterController == nullptr) {
        Serial.println("❌ SorterController not initialized");
        return false;
    }
    
    return _sorterController->checkConnection();
}