#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>

// State constants for conveyor
extern const String CONVEYOR_STATE_STOPPED;
extern const String CONVEYOR_STATE_RUNNING;
extern const String CONVEYOR_STATE_ERROR;

class Conveyor {
public:
    void initialize();
    void start();
    void stop();
    String getState() const;
    void setState(String state);

    
    // State conversion function for automatic monitoring
    static String stateToString(String state);
    
    // Module self-configuration
    static void setupStateRules(StateErrorManager& stateErrorManager);
    
    // State monitoring configuration
    static void configureStateMonitoring(StateErrorManager& stateErrorManager, String* stateVariable);
    
    // Module initialization with automatic state monitoring setup
    void initializeWithStateMonitoring(StateErrorManager& stateErrorManager);
    
    // Setup physical state checks for automatic error detection
    static void setupPhysicalChecks(StateErrorManager& stateErrorManager);

private:
    String conveyorState;
};

#endif