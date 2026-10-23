// Scheduler: the state machine of the system.
// WAITING -> CHECK_CONNECT -> INIT_POSITIONS -> PRODUCTION, plus ERROR.

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

// Runs the system state machine.
// executeCurrentState() is called on every loop pass and does the work of the current state.
class Scheduler {
public:
    // Sets the state to WAITING.
    void initialize();
    // Changes the state and logs the change.
    void setState(SchedulerState state);
    SchedulerState getState();
    // Starts the checks: WAITING -> CHECK_CONNECT.
    void triggerCheckConnect();
    // Starts the position step: -> INIT_POSITIONS.
    void triggerInitPositions();
    // Starts production: -> PRODUCTION.
    void triggerProduction();
    // Stops the system: back to WAITING and the conveyor stopped (button B, or a failed step).
    void triggerStop();
    
    // Dependency configuration (MUST be called after initialize)
    // Gives the scheduler the modules it drives. Called once from setup().
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