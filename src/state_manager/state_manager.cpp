// StateManager: detects state changes, applies the rules, and warns about stuck states (see state_manager.h).

#include "state_manager.h"
#include "log.h"
#include "error.h"


void StateManager::addStateRule(String module, String fromState, String toState, ErrorLevel level, String message)
{
    stateRules.push_back({module, fromState, toState, level, message});
}

// Runs every rule that matches this transition. '*' in a rule matches any state.
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

// Only the conveyor is handled here. An ALARM stops production; a FATAL alarm also goes to ERROR.
void StateManager::handleCriticalStateChange(String module, String newState, ErrorLevel level)
{
    if (_scheduler == nullptr) {
        return; // No scheduler configured, do nothing
    }

    // Handle critical states per module
    if (module == "Conveyor") {
        if (newState == "ALARM") {
            LOG_ERROR("StateManager", "conveyor ALARM detected");
            
            // If we are in production, switch to WAITING
            if (_scheduler->getState() == SchedulerState::Production) {
                LOG_WARN("StateManager", "stopping production on conveyor alarm");
                _scheduler->setState(SchedulerState::Waiting);
            }
            
            // For fatal errors, switch to ERROR
            if (level == ERROR_FATAL) {
                LOG_ERROR("StateManager", "fatal error, switching to ERROR");
                _scheduler->setState(SchedulerState::Error);
            }
        }
    }
}

void StateManager::setScheduler(Scheduler* scheduler)
{
    _scheduler = scheduler;
}

// States a module is expected to stay in for a long time (idle, or running normally)
static bool isSteadyState(const String &state)
{
    return state == "WAITING" || state == "STOPPED" || state == "INITIAL" ||
           state == "IDLE" || state == "RUNNING" || state == "PRODUCTION";
}

// Called on every loop pass. Each observed state is compared with the value seen last time.
void StateManager::monitorAllStates()
{
    unsigned long currentTime = millis();
    for (StateMonitor &monitor : stateMonitors)
    {
        // 1. Read the current state through the monitor
        String currentState = monitor.readState();

        // 2. Compare with the previous state
        if (currentState != monitor.previousState)
        {
            // 3. CHANGE DETECTED!
            LOG_INFO("StateManager", "%s: %s -> %s", monitor.moduleName.c_str(), monitor.previousState.c_str(), currentState.c_str());

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

                // Only warn about transitional states: steady operating states are expected to last
                if (!isSteadyState(currentState))
                {
                    String errorMessage = String(monitor.moduleName) + " stuck in state: " + currentState;
                    Error::triggerError(errorMessage, ERROR_WARNING, monitor.moduleName);
                    monitor.isStuckWarningGiven = true;
                }
            }
        }
    }
}
