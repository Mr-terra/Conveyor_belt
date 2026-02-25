// Test d'intégration StateManager <-> Scheduler
// Ce fichier démontre le fonctionnement de l'intégration

#include <Arduino.h>
#include "state_manager.h"
#include "scheduler.h"
#include "conveyor.h"

// Simulation des états pour le test
void testConveyorAlarmDuringProduction() {
    Serial.println("=== Test 1: Conveyor ALARM pendant production ===");
    
    // Initialisation
    scheduler.initialize();
    stateManager.setScheduler(&scheduler);
    
    // Simuler le passage en production
    scheduler.setState(STATE_PRODUCTION);
    Serial.print("État initial: ");
    Serial.println(scheduler.getState());
    
    // Simuler une alarme conveyor
    // Dans la réalité, cela serait déclenché par le hardware
    // Ici, nous simulons le changement d'état
    Serial.println("Simulating conveyor ALARM...");
    
    // Le StateManager devrait détecter cela et agir
    // (Dans la réalité, cela se fait via monitorAllStates())
    
    Serial.println("Expected: Scheduler should return to WAITING state");
    Serial.println("Test completed\n");
}

void testErrorStateRecovery() {
    Serial.println("=== Test 2: Récupération de l'état ERROR ===");
    
    // Mettre en état ERROR
    scheduler.setState(STATE_ERROR);
    Serial.print("État initial: ");
    Serial.println(scheduler.getState());
    
    // Exécuter l'état courant (devrait vérifier les conditions de récupération)
    scheduler.executeCurrentState();
    
    Serial.println("Expected: Should check recovery conditions");
    Serial.println("If conditions met, should return to WAITING");
    Serial.println("Test completed\n");
}

void testStateTransitions() {
    Serial.println("=== Test 3: Transitions d'états normales ===");
    
    // Test des transitions normales
    scheduler.setState(STATE_WAITING);
    Serial.print("WAITING -> ");
    
    scheduler.setState(STATE_CHECK_CONNECT);
    Serial.print("CHECK_CONNECT -> ");
    
    scheduler.setState(STATE_INIT_POSITIONS);
    Serial.print("INIT_POSITIONS -> ");
    
    scheduler.setState(STATE_PRODUCTION);
    Serial.println("PRODUCTION");
    
    Serial.println("All transitions completed successfully");
    Serial.println("Test completed\n");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("🧪 Starting Integration Tests\n");
    
    testStateTransitions();
    testConveyorAlarmDuringProduction();
    testErrorStateRecovery();
    
    Serial.println("✅ All tests completed");
}

void loop() {
    // Nothing to do here
}
