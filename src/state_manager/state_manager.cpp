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
            
            // Handle critical state changes
            handleCriticalStateChange(module, toState, rule.level);
        }
    }
}

void StateManager::handleCriticalStateChange(String module, String newState, ErrorLevel level)
{
    if (_scheduler == nullptr) {
        return; // No scheduler configured, do nothing
    }

    // Handle critical states per module
    if (module == "Conveyor") {
        if (newState == "ALARM") {
            Serial.println("🚨 Critical conveyor ALARM detected");
            
            // If we are in production, switch to WAITING
            if (_scheduler->getState() == STATE_PRODUCTION) {
                Serial.println("   Stopping production due to conveyor alarm");
                _scheduler->setState(STATE_WAITING);
            }
            
            // For fatal errors, switch to ERROR
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
        // 1. Read the current state through the pointer
        String *statePtr = static_cast<String *>(monitor.stateVariable);
        String currentState = monitor.stateToString(statePtr);

        // 2. Compare with the previous state
        if (currentState != monitor.previousState)
        {
            // 3. CHANGE DETECTED!
            Serial.print("🔄 State change detected: ");
            Serial.print(monitor.moduleName);
            Serial.print(" from ");
            Serial.print(monitor.previousState);
            Serial.print(" to ");
            Serial.println(currentState);

            // 4. Check the rules
            checkStateRules(monitor.moduleName, monitor.previousState, currentState);

            // 5. Update the previous state
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