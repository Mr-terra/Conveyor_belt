#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>

// Forward declaration
class StateManager;

// State constants for conveyor
extern const String CONVEYOR_STATE_STOPPED;
extern const String CONVEYOR_STATE_RUNNING;
extern const String CONVEYOR_STATE_ERROR;

class Conveyor {
public:
    void initialize(StateManager& stateManager);
    void start();
    void stop();
    String getState() const;
    void setState(String state);

    
    // State conversion function for automatic monitoring
    static String stateToString(String state);
    
    // Module self-configuration
    static void setupStateRules(StateManager& stateManager);
    
    // State monitoring configuration
    static void configureStateMonitoring(StateManager& stateManager, String* stateVariable);

private:
    String conveyorState;
};

#endif