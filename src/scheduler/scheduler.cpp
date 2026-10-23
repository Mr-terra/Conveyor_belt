// Scheduler: the state transitions and the work done in each state (see scheduler.h).

#include "scheduler.h"
#include "log.h"
#include "main_sorter.h"
#include "check_connect.h"
#include "init_position.h"
#include "conveyor.h"
#include <Arduino.h>


// Name of each state, used on the screen, in the logs and by the state rules.
String Scheduler::stateToString(SchedulerState state)
{
    switch (state)
    {
    case SchedulerState::Waiting:
        return "WAITING";
    case SchedulerState::CheckConnect:
        return "CHECK_CONNECT";
    case SchedulerState::InitPositions:
        return "INIT_POSITIONS";
    case SchedulerState::Production:
        return "PRODUCTION";
    case SchedulerState::Error:
        return "ERROR";
    }
    return "UNKNOWN";
}

// Starts in WAITING: nothing is driven until Start is pressed.
void Scheduler::initialize() {
    _current_state = SchedulerState::Waiting;
    LOG_INFO("Scheduler", "initialized, state WAITING");
}

// Stores the modules the scheduler drives. Called once from setup().
void Scheduler::setDependencies(
    CheckConnect* checkConnect,
    InitPosition* initPosition,
    SorterController* sorterController,
    Conveyor* conveyor
) {
    _checkConnect = checkConnect;
    _initPosition = initPosition;
    _sorterController = sorterController;
    _conveyor = conveyor;
}

void Scheduler::setState(SchedulerState state) {
    _current_state = state;
    LOG_INFO("Scheduler", "state -> %s", stateToString(state).c_str());
}

SchedulerState Scheduler::getState() {
    return _current_state;
}

void Scheduler::triggerCheckConnect()
{
    if (_checkConnect == nullptr) {
        LOG_ERROR("Scheduler", "cannot start CHECK_CONNECT: CheckConnect not configured");
        setState(SchedulerState::Error);
        return;
    }
    setState(SchedulerState::CheckConnect);
}

void Scheduler::triggerInitPositions()
{
    if (_initPosition == nullptr) {
        LOG_ERROR("Scheduler", "cannot start INIT_POSITIONS: InitPosition not configured");
        setState(SchedulerState::Error);
        return;
    }
    setState(SchedulerState::InitPositions);
}

void Scheduler::triggerProduction()
{
    if (_sorterController == nullptr || _conveyor == nullptr) {
        LOG_ERROR("Scheduler", "cannot start PRODUCTION: dependencies not configured");
        setState(SchedulerState::Error);
        return;
    }
    setState(SchedulerState::Production);
}

// Back to WAITING with the conveyor stopped. Used by button B and by every failed step.
void Scheduler::triggerStop()
{
    setState(SchedulerState::Waiting);
    if (_conveyor != nullptr) {
        _conveyor->stop();
    }
}

// Main method that runs the logic of the current state
void Scheduler::executeCurrentState() {
    // ERROR state is handled first
    if (_current_state == SchedulerState::Error) {
        // Check whether we can leave the ERROR state
        if (canRecoverFromError()) {
            LOG_INFO("Scheduler", "error conditions resolved, back to WAITING");
            Error::clearCriticalErrors();
            setState(SchedulerState::Waiting);
        }
        return; // Do not run other states in ERROR mode
    }
    // Each step below runs once per pass. It moves on when it succeeds, and goes back to WAITING when it fails.
    if (_current_state == SchedulerState::CheckConnect) {
        if (_checkConnect != nullptr) {
            bool connectionsOk = _checkConnect->checkConnections();
            if (connectionsOk) {
                LOG_INFO("Scheduler", "connections OK, starting INIT_POSITIONS");
                triggerInitPositions();
            } else {
                LOG_ERROR("Scheduler", "connection check failed, back to WAITING");
                triggerStop();
            }
        } else {
            LOG_ERROR("Scheduler", "CheckConnect not configured, back to WAITING");
            triggerStop();
        }
    }
    else if (_current_state == SchedulerState::InitPositions) {
        if (_initPosition != nullptr) {
            bool initOk = _initPosition->initializePositions();
            if (initOk) {
                LOG_INFO("Scheduler", "position initialization OK, starting PRODUCTION");
                triggerProduction();
            } else {
                LOG_ERROR("Scheduler", "position initialization failed, back to WAITING");
                triggerStop();
            }
        } else {
            LOG_ERROR("Scheduler", "InitPosition not configured, back to WAITING");
            triggerStop();
        }
    }
    else if (_current_state == SchedulerState::Production) {
        // Check the conveyor state before continuing
        // Safety check: production stops if the conveyor is in alarm.
        // The state rules normally stop production first, so this is a second guard.
        if (_conveyor != nullptr && _conveyor->getState() == ConveyorState::Alarm) {
            LOG_WARN("Scheduler", "conveyor in ALARM during production, stopping");
            setState(SchedulerState::Waiting);
            return;
        }
        if (_conveyor != nullptr) {
            _conveyor->move(true); // Mouvement continu
        }
        if (_sorterController != nullptr) {
            _sorterController->production();
        }
    }
    else if (_current_state == SchedulerState::Waiting) {
        // In the WAITING state, the system is ready but does nothing (no log: runs every tick)
    }
}

// Check whether we can leave the ERROR state
bool Scheduler::canRecoverFromError() {
    if (_checkConnect == nullptr || _conveyor == nullptr) {
        return false;
    }
    
    // A critical error stays latched until the recovery succeeds
    if (Error::hasCriticalErrors()) {
        return false;
    }

    // Check that the connections are OK
    bool connectionsOk = _checkConnect->checkConnections();
    if (!connectionsOk) {
        return false;
    }
    
    // Check that the conveyor is not in ALARM
    if (_conveyor->getState() == ConveyorState::Alarm) {
        return false;
    }
    return true;
}
