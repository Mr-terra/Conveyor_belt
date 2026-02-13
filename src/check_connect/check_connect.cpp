#include "check_connect.h"
#include "conveyor.h"
#include "main_sorter.h"
#include "reader.h"
#include <Arduino.h>

CheckConnect checkConnect;

void CheckConnect::initialize() {
    Serial.println("CheckConnect initialized");
}

bool CheckConnect::checkConnections() {
    Serial.println("\n🔍 === Checking all connections ===");
    
    // Check all connections via static methods
    bool dolibarrOk = checkDolibarrConnection();
    bool motorOk = checkMotorConnection();
    bool sorterOk = checkSorterConnection();
    bool rfidOk = checkRFIDConnection();

    bool allOk = dolibarrOk && motorOk && sorterOk && rfidOk;
    
    if (allOk) {
        Serial.println("✅ === All connections OK ===\n");
    } else {
        Serial.println("❌ === Connection check FAILED ===\n");
    }
    
    return allOk;
}

bool CheckConnect::checkDolibarrConnection() {
    // TODO: Implémenter le test réseau/API Dolibarr
    Serial.println("🌐 Checking Dolibarr connection...");
    // Simulation pour l'instant
    Serial.println("⚠️  Dolibarr check not implemented (assumed OK)");
    return true;
}

bool CheckConnect::checkMotorConnection() {
    Serial.println("🔧 Checking Motor connection...");
    return Conveyor::checkConnection();
}

bool CheckConnect::checkSorterConnection() {
    Serial.println("📦 Checking Sorter connection...");
    return SorterController::checkConnection();
}

bool CheckConnect::checkRFIDConnection() {
    Serial.println("📡 Checking RFID connection...");
    return RFIDReader::checkConnection();
}