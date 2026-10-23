#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include "error.h"
#include "scheduler.h"

class StateManager {
public:
    // Pure state observation - no business logic dependencies
    template<typename T>
    void observeState(T* stateVariable, String moduleName, String (*stateToString)(T));
    
    void addStateRule(String module, String fromState, String toState, ErrorLevel level, String message);
    
    // Automatic state monitoring - pure observation
    void monitorAllStates();
    
    // Module self-configuration interface
    template<typename T>
    void configureModuleStateMonitoring(T* stateVariable, String moduleName, String (*stateToString)(T), 
                                      void (*setupRules)(StateManager&));
    
    // Set scheduler reference for state coordination
    void setScheduler(Scheduler* scheduler);

private:
    bool hasCriticalErrorFlag = false;
    Scheduler* _scheduler = nullptr; // Reference to scheduler for state coordination
    
    struct StateRule {
        String module;
        String fromState;
        String toState;
        ErrorLevel level;
        String message;
    };
    std::vector<StateRule> stateRules;
    
    // State monitoring structures - pure data
    struct StateMonitor {
        std::function<String()> readState; // Returns the formatted current state
        String moduleName;
        String previousState;
        unsigned long lastChangeTime;
        bool isStuckWarningGiven;
    };
    std::vector<StateMonitor> stateMonitors;
    
    unsigned long stuckStateTimeout = 5000; // 5 seconds
    
    void checkStateRules(String module, String fromState, String toState);
    
    // Template implementation for state observation
    template<typename T>
    void addStateMonitor(T* stateVariable, String moduleName, String (*stateToString)(T));

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