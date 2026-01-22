
#include "state_error_manager.h"

StateErrorManager stateErrorManager;

void StateErrorManager::triggerError(String message, ErrorLevel level, String module) {
    Error error(message, level, module);
    logError(error);
    
    // Pure observation - delegate actions to callbacks
    if (errorCallback) {
        errorCallback(error);
    }
    
    if (level == ERROR_CRITICAL || level == ERROR_FATAL) {
        hasCriticalErrorFlag = true;
        if (criticalErrorCallback) {
            criticalErrorCallback();
        }
    }
}

bool StateErrorManager::hasCriticalErrors() const {
    return hasCriticalErrorFlag;
}

void StateErrorManager::clearCriticalErrors() {
    hasCriticalErrorFlag = false;
}

void StateErrorManager::logError(const Error& error) {
    Serial.println(error.toString());
}

void StateErrorManager::setErrorCallback(ErrorCallback callback) {
    errorCallback = callback;
}

void StateErrorManager::setCriticalErrorCallback(CriticalErrorCallback callback) {
    criticalErrorCallback = callback;
}

// Physical state verification structure
struct PhysicalCheck {
    String module;
    String state;
    bool (*checkFunction)();
    String errorMessage;
    ErrorLevel level;
};

std::vector<PhysicalCheck> physicalChecks;

// Automatic monitoring control
void StateErrorManager::startAutomaticMonitoring() {
    Serial.println("StateErrorManager: Automatic monitoring started");
    // Monitoring is now automatic - no manual intervention needed
}

void StateErrorManager::addPhysicalCheck(String module, String state, bool (*checkFunction)(), String errorMessage, ErrorLevel level) {
    physicalChecks.push_back({module, state, checkFunction, errorMessage, level});
}

void StateErrorManager::stopAutomaticMonitoring() {
    Serial.println("StateErrorManager: Automatic monitoring stopped");
}

bool StateErrorManager::isAutomaticMonitoringActive() const {
    return true; // Always active in this architecture
}

// Module self-configuration interface
template<typename T>
void StateErrorManager::configureModuleStateMonitoring(T* stateVariable, String moduleName, String (*stateToString)(T), 
                                                      void (*setupRules)(StateErrorManager&)) {
    // Register the state variable for monitoring
    observeState(stateVariable, moduleName, stateToString);
    
    // Let the module setup its own rules
    if (setupRules) {
        setupRules(*this);
    }
}

// Template implementation for adding state monitors
template<typename T>
void StateErrorManager::addStateMonitor(T* stateVariable, String moduleName, String (*stateToString)(T)) {
    // Cast the function pointer to void* for storage
    String (*castedFunc)(void*) = reinterpret_cast<String (*)(void*)>(stateToString);
    
    StateMonitor monitor;
    monitor.stateVariable = stateVariable;
    monitor.moduleName = moduleName;
    monitor.stateToString = castedFunc;
    monitor.previousState = stateToString(*stateVariable);
    monitor.lastChangeTime = millis();
    monitor.isStuckWarningGiven = false;
    
    stateMonitors.push_back(monitor);
}

// Template implementation for observing states
template<typename T>
void StateErrorManager::observeState(T* stateVariable, String moduleName, String (*stateToString)(T)) {
    addStateMonitor(stateVariable, moduleName, stateToString);
}

void StateErrorManager::addStateRule(String module, String fromState, String toState, ErrorLevel level, String message) {
    stateRules.push_back({module, fromState, toState, level, message});
}

void StateErrorManager::addForbiddenTransition(String module, String fromState, String toState, String message) {
    forbiddenTransitions.push_back({module, fromState, toState, message});
}

void StateErrorManager::checkStateRules(String module, String fromState, String toState) {
    for (const StateRule& rule : stateRules) {
        if (rule.module == module &&
            (rule.fromState == fromState || rule.fromState == "*") &&
            (rule.toState == toState || rule.toState == "*")) {
            triggerError(rule.message, rule.level, module);
        }
    }
}

void StateErrorManager::checkForbiddenTransitions(String module, String fromState, String toState) {
    for (const ForbiddenTransition& transition : forbiddenTransitions) {
        if (transition.module == module &&
            transition.fromState == fromState &&
            transition.toState == toState) {
            triggerError(transition.message, ERROR_CRITICAL, module);
        }
    }
}

void StateErrorManager::monitorAllStates() {
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
            checkForbiddenTransitions(monitor.moduleName, monitor.previousState, currentState);
            
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
                    triggerError(errorMessage, ERROR_WARNING, monitor.moduleName);
                    monitor.isStuckWarningGiven = true;
                }
            }
        }
        
        // Perform physical state checks for current state
        checkPhysicalState(monitor.moduleName, currentState);
    }
}

void StateErrorManager::checkPhysicalState(String module, String currentState) {
    for (const PhysicalCheck& check : physicalChecks) {
        if (check.module == module && check.state == currentState) {
            if (!check.checkFunction()) {
                triggerError(check.errorMessage, check.level, module);
            }
        }
    }
}