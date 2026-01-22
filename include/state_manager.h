#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include "error.h"

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

private:
    bool hasCriticalErrorFlag = false;
    
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
        void* stateVariable;
        String moduleName;
        String (*stateToString)(void*);
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
};

// Template implementations MUST be in header file
template<typename T>
void StateManager::addStateMonitor(T* stateVariable, String moduleName, String (*stateToString)(T)) {
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