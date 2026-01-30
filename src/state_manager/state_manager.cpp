
#include "state_manager.h"
#include "error.h"

StateManager stateManager;

// Physical state verification structure
struct PhysicalCheck {
    String module;
    String state;
    bool (*checkFunction)();
    String errorMessage;
    ErrorLevel level;
};

void StateManager::addStateRule(String module, String fromState, String toState, ErrorLevel level, String message) {
    stateRules.push_back({module, fromState, toState, level, message});
}

void StateManager::checkStateRules(String module, String fromState, String toState) {
    for (const StateRule& rule : stateRules) {
        if (rule.module == module &&
            (rule.fromState == fromState || rule.fromState == "*") &&
            (rule.toState == toState || rule.toState == "*")) {
            Error::triggerError(rule.message, rule.level, module);
        }
    }
}

void StateManager::monitorAllStates() {
    unsigned long currentTime = millis();
    
    for (StateMonitor& monitor : stateMonitors) {
        // Get current state using the stored function pointer
        String (*stateToStringFunc)(void*) = monitor.stateToString;
        String currentState;
        
        // Safe type handling - no direct object dependencies
        String* statePtr = static_cast<String*>(monitor.stateVariable);
        currentState = stateToStringFunc(statePtr);
        
        if (currentState != monitor.previousState) {
            Serial.print("State change detected: ");
            Serial.print(monitor.moduleName);
            Serial.print(" from ");
            Serial.print(monitor.previousState);
            Serial.print(" to ");
            Serial.println(currentState);
            
            // Check both state rules and forbidden transitions
            checkStateRules(monitor.moduleName, monitor.previousState, currentState);
            
            monitor.previousState = currentState;
            monitor.lastChangeTime = currentTime;
            monitor.isStuckWarningGiven = false;
        } else {
            // Check for stuck states
            if (!monitor.isStuckWarningGiven && 
                currentTime - monitor.lastChangeTime > stuckStateTimeout) {
                
                // Only warn about non-terminal states
                if (currentState != "WAITING" && currentState != "STOPPED" && currentState != "INITIAL") {
                    String errorMessage = String(monitor.moduleName) + " stuck in state: " + currentState;
                    Error::triggerError(errorMessage, ERROR_WARNING, monitor.moduleName);
                    monitor.isStuckWarningGiven = true;
                }
            }
        }
    }
}