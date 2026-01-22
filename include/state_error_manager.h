#ifndef STATE_ERROR_MANAGER_H
#define STATE_ERROR_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include "error.h"

class StateErrorManager {
public:
    // Error management - pure observation and reporting
    void triggerError(String message, ErrorLevel level, String module);
    bool hasCriticalErrors() const;
    void clearCriticalErrors();

    // Pure state observation - no business logic dependencies
    template<typename T>
    void observeState(T* stateVariable, String moduleName, String (*stateToString)(T));
    
    void addStateRule(String module, String fromState, String toState, ErrorLevel level, String message);
    void addForbiddenTransition(String module, String fromState, String toState, String message);
    
    // Automatic state monitoring - pure observation
    void monitorAllStates();
    
    // Callback system for error handling (separation of concerns)
    using ErrorCallback = std::function<void(const Error& error)>;
    using CriticalErrorCallback = std::function<void()>;
    
    void setErrorCallback(ErrorCallback callback);
    void setCriticalErrorCallback(CriticalErrorCallback callback);
    
    // Module self-configuration interface
    template<typename T>
    void configureModuleStateMonitoring(T* stateVariable, String moduleName, String (*stateToString)(T), 
                                      void (*setupRules)(StateErrorManager&));
    
    // Automatic monitoring control
    void startAutomaticMonitoring();
    void stopAutomaticMonitoring();
    bool isAutomaticMonitoringActive() const;
    
    // Physical state verification functions (for automatic error detection)
    void addPhysicalCheck(String module, String state, bool (*checkFunction)(), String errorMessage, ErrorLevel level);

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
    
    struct ForbiddenTransition {
        String module;
        String fromState;
        String toState;
        String message;
    };
    std::vector<ForbiddenTransition> forbiddenTransitions;
    
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
    
    // Callback functions
    ErrorCallback errorCallback;
    CriticalErrorCallback criticalErrorCallback;
    
    void checkStateRules(String module, String fromState, String toState);
    void checkForbiddenTransitions(String module, String fromState, String toState);
    void checkPhysicalState(String module, String currentState);
    void logError(const Error& error);
    
    // Template implementation for state observation
    template<typename T>
    void addStateMonitor(T* stateVariable, String moduleName, String (*stateToString)(T));
};

#endif