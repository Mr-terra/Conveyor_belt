#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

// Forward declarations to avoid circular includes
class CheckConnect;
class InitPosition;
class SorterController;
class Conveyor;

// State constants for scheduler
extern const String STATE_WAITING;
extern const String STATE_CHECK_CONNECT;
extern const String STATE_INIT_POSITIONS;
extern const String STATE_PRODUCTION;
extern const String STATE_ERROR;

class Scheduler {
public:
    void initialize();
    void setState(String state);
    String getState();
    void triggerCheckConnect();
    void triggerInitPositions();
    void triggerProduction();
    void triggerStop();
    
    // Dependency configuration (MUST be called after initialize)
    void setDependencies(
        CheckConnect* checkConnect,
        InitPosition* initPosition,
        SorterController* sorterController,
        Conveyor* conveyor
    );
    
    // Main method that runs the logic of the current state
    void executeCurrentState();
    
    // State conversion function for automatic monitoring
    static String stateToString(String state);

private:
    String _current_state;
    bool canRecoverFromError(); // Checks whether the error conditions have been resolved
};

#endif