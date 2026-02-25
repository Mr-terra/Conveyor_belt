#include "state_manager.h"
#include "error.h"

StateManager stateManager;

// Physical state verification structure
struct PhysicalCheck
{
    String module;
    String state;
    bool (*checkFunction)();
    String errorMessage;
    ErrorLevel level;
};

void StateManager::addStateRule(String module, String fromState, String toState, ErrorLevel level, String message)
{
    stateRules.push_back({module, fromState, toState, level, message});
}

void StateManager::checkStateRules(String module, String fromState, String toState)
{
    for (const StateRule &rule : stateRules)
    {
        if (rule.module == module &&
            (rule.fromState == fromState || rule.fromState == "*") &&
            (rule.toState == toState || rule.toState == "*"))
        {
            Error::triggerError(rule.message, rule.level, module);
            
            // Gérer les changements d'état critiques
            handleCriticalStateChange(module, toState, rule.level);
        }
    }
}

void StateManager::handleCriticalStateChange(String module, String newState, ErrorLevel level)
{
    if (_scheduler == nullptr) {
        return; // Pas de scheduler configuré, ne rien faire
    }

    // Gestion des états critiques par module
    if (module == "Conveyor") {
        if (newState == "ALARM") {
            Serial.println("🚨 Critical conveyor ALARM detected");
            
            // Si nous sommes en production, passer en WAITING
            if (_scheduler->getState() == STATE_PRODUCTION) {
                Serial.println("   Stopping production due to conveyor alarm");
                _scheduler->setState(STATE_WAITING);
            }
            
            // Pour les erreurs fatales, passer en ERROR
            if (level == ERROR_FATAL) {
                Serial.println("   Fatal error - switching to ERROR state");
                _scheduler->setState(STATE_ERROR);
            }
        }
    }
}

void StateManager::setScheduler(Scheduler* scheduler)
{
    _scheduler = scheduler;
    Serial.println("✅ StateManager configured with Scheduler reference");
}

void StateManager::monitorAllStates()
{
    unsigned long currentTime = millis();
    for (StateMonitor &monitor : stateMonitors)
    {
        // 1. Lire l'état actuel via le pointeur
        String *statePtr = static_cast<String *>(monitor.stateVariable);
        String currentState = monitor.stateToString(statePtr);

        // 2. Comparer avec l'état précédent
        if (currentState != monitor.previousState)
        {
            // 3. CHANGEMENT DÉTECTÉ !
            Serial.print("🔄 State change detected: ");
            Serial.print(monitor.moduleName);
            Serial.print(" from ");
            Serial.print(monitor.previousState);
            Serial.print(" to ");
            Serial.println(currentState);

            // 4. Vérifier les règles
            checkStateRules(monitor.moduleName, monitor.previousState, currentState);

            // 5. Mettre à jour l'état précédent
            monitor.previousState = currentState;
            monitor.lastChangeTime = currentTime;
            monitor.isStuckWarningGiven = false;
        }
        else
        {
            // Check for stuck states
            if (!monitor.isStuckWarningGiven &&
                currentTime - monitor.lastChangeTime > stuckStateTimeout)
            {

                // Only warn about non-terminal states
                if (currentState != "WAITING" && currentState != "STOPPED" && currentState != "INITIAL")
                {
                    String errorMessage = String(monitor.moduleName) + " stuck in state: " + currentState;
                    Error::triggerError(errorMessage, ERROR_WARNING, monitor.moduleName);
                    monitor.isStuckWarningGiven = true;
                }
            }
        }
    }
}