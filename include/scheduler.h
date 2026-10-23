#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

// Forward declarations to avoid circular includes
class CheckConnect;
class InitPosition;
class SorterController;
class Conveyor;

// States of the scheduler. Scheduler::stateToString() gives the name used on the screen and in the logs.
enum class SchedulerState : uint8_t
{
    Waiting,
    CheckConnect,
    InitPositions,
    Production,
    Error
};

class Scheduler {
public:
    void initialize();
    void setState(SchedulerState state);
    SchedulerState getState();
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
    static String stateToString(SchedulerState state);

private:
    SchedulerState _current_state = SchedulerState::Waiting;

    // Dependencies injected through setDependencies()
    CheckConnect* _checkConnect = nullptr;
    InitPosition* _initPosition = nullptr;
    SorterController* _sorterController = nullptr;
    Conveyor* _conveyor = nullptr;
    bool canRecoverFromError(); // Checks whether the error conditions have been resolved
};

#endif