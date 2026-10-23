// StateManager: watches the state of the modules, applies the rules, and reports stuck states.

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include "error.h"
#include "scheduler.h"

// Observes state variables of the modules and applies rules when they change.
// A module registers its state variable, a function that names each state, and its rules.
// monitorAllStates() is called on every loop pass.
class StateManager {
public:
    // Pure state observation - no business logic dependencies
    template<typename T>
    // Starts observing a state variable under the module name moduleName.
    void observeState(T* stateVariable, String moduleName, String (*stateToString)(T));
    
    // Adds a rule: when module goes from fromState to toState, report message at level.
    // '*' matches any state.
    void addStateRule(String module, String fromState, String toState, ErrorLevel level, String message);
    
    // Automatic state monitoring - pure observation
    // Compares every observed state with the last value seen.
    // Applies the rules on a change, and warns when a transitional state lasts too long.
    void monitorAllStates();
    
    // Module self-configuration interface
    template<typename T>
    void configureModuleStateMonitoring(T* stateVariable, String moduleName, String (*stateToString)(T), 
                                      void (*setupRules)(StateManager&));
    
    // Set scheduler reference for state coordination
    // Lets the rules act on the scheduler (stop production, or go to ERROR).
    void setScheduler(Scheduler* scheduler);

private:
    bool hasCriticalErrorFlag = false;
    Scheduler* _scheduler = nullptr; // Reference to scheduler for state coordination
    
    // One rule, see addStateRule().
    struct StateRule {
        String module;
        String fromState;
        String toState;
        ErrorLevel level;
        String message;
    };
    std::vector<StateRule> stateRules;
    
    // State monitoring structures - pure data
    // One observed state variable, and the last value seen.
    struct StateMonitor {
        std::function<String()> readState; // Returns the formatted current state
        String moduleName;
        String previousState;
        unsigned long lastChangeTime;
        bool isStuckWarningGiven;
    };
    std::vector<StateMonitor> stateMonitors;
    
    // How long a transitional state may last before a warning is raised, in milliseconds.
    unsigned long stuckStateTimeout = 5000; // 5 seconds
    
    void checkStateRules(String module, String fromState, String toState);
    
    // Template implementation for state observation
    template<typename T>
    void addStateMonitor(T* stateVariable, String moduleName, String (*stateToString)(T));

    // Reacts to a change that matched a rule: stops production or goes to ERROR, depending on
    // the module, the new state and the level.
    void handleCriticalStateChange(String module, String newState, ErrorLevel level);
};

// Template implementations MUST be in header file
template<typename T>
void StateManager::addStateMonitor(T* stateVariable, String moduleName, String (*stateToString)(T)) {
    StateMonitor monitor;
    monitor.readState = [stateVariable, stateToString]() { return stateToString(*stateVariable); };
    monitor.moduleName = moduleName;
    monitor.previousState = monitor.readState();
    monitor.lastChangeTime = millis();
    monitor.isStuckWarningGiven = false;
    
    stateMonitors.push_back(monitor);
}

template<typename T>
void StateManager::observeState(T* stateVariable, String moduleName, String (*stateToString)(T)) {
    addStateMonitor(stateVariable, moduleName, stateToString);
}

template<typename T>
void StateManager::configureModuleStateMonitoring(T* stateVariable, String moduleName, String (*stateToString)(T), 
                                                      void (*setupRules)(StateManager&)) {
    observeState(stateVariable, moduleName, stateToString);
    
    if (setupRules) {
        setupRules(*this);
    }
}

#endif