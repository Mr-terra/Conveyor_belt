#include "scheduler.h"
#include "main_sorter.h"
#include "check_connect.h"
#include "init_position.h"
#include "conveyor.h"
#include <Arduino.h>

Scheduler scheduler;

// State constants for scheduler (now declared in header)
// State conversion function for automatic monitoring
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

void Scheduler::initialize() {
    _current_state = SchedulerState::Waiting;
    Serial.println("Scheduler initialized. State: WAITING");
}

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
    Serial.print("Scheduler state changed to: ");
    Serial.println(stateToString(state));
}

SchedulerState Scheduler::getState() {
    return _current_state;
}

void Scheduler::triggerCheckConnect()
{
    if (_checkConnect == nullptr) {
        Serial.println("❌ Cannot trigger CHECK_CONNECT: CheckConnect not configured");
        setState(SchedulerState::Error);
        return;
    }
    setState(SchedulerState::CheckConnect);
}

void Scheduler::triggerInitPositions()
{
    if (_initPosition == nullptr) {
        Serial.println("❌ Cannot trigger INIT_POSITIONS: InitPosition not configured");
        setState(SchedulerState::Error);
        return;
    }
    setState(SchedulerState::InitPositions);
}

void Scheduler::triggerProduction()
{
    if (_sorterController == nullptr || _conveyor == nullptr) {
        Serial.println("❌ Cannot trigger PRODUCTION: Dependencies not configured");
        setState(SchedulerState::Error);
        return;
    }
    setState(SchedulerState::Production);
}

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
        Serial.println("⚠️  System in ERROR state - checking recovery conditions");
        
        // Check whether we can leave the ERROR state
        if (canRecoverFromError()) {
            Serial.println("🔧 Error conditions resolved - returning to WAITING");
            Error::clearCriticalErrors();
            setState(SchedulerState::Waiting);
        } else {
            Serial.println("🚫 Still in ERROR state - manual intervention may be required");
        }
        return; // Do not run other states in ERROR mode
    }
    
    if (_current_state == SchedulerState::CheckConnect) {
        Serial.println("🔍 Executing CHECK_CONNECT state");
        if (_checkConnect != nullptr) {
            bool connectionsOk = _checkConnect->checkConnections();
            if (connectionsOk) {
                Serial.println("✅ Connections OK - transitioning to INIT_POSITIONS");
                triggerInitPositions();
            } else {
                Serial.println("❌ Connection check failed - returning to WAITING");
                triggerStop();
            }
        } else {
            Serial.println("❌ CheckConnect not configured - returning to WAITING");
            triggerStop();
        }
    }
    else if (_current_state == SchedulerState::InitPositions) {
        Serial.println("🔧 Executing INIT_POSITIONS state");
        if (_initPosition != nullptr) {
            bool initOk = _initPosition->initializePositions();
            if (initOk) {
                Serial.println("✅ Position initialization OK - transitioning to PRODUCTION");
                triggerProduction();
            } else {
                Serial.println("❌ Position initialization failed - returning to WAITING");
                triggerStop();
            }
        } else {
            Serial.println("❌ InitPosition not configured - returning to WAITING");
            triggerStop();
        }
    }
    else if (_current_state == SchedulerState::Production) {
        // Check the conveyor state before continuing
        if (_conveyor != nullptr && _conveyor->getState() == "ALARM") {
            Serial.println("❌ Conveyor in ALARM state during production - stopping");
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
        Serial.println("   Cannot recover - dependencies not configured");
        return false;
    }
    
    // A critical error stays latched until the recovery succeeds
    if (Error::hasCriticalErrors()) {
        Serial.println("   Cannot recover - a critical error is still latched");
        return false;
    }

    // Check that the connections are OK
    bool connectionsOk = _checkConnect->checkConnections();
    if (!connectionsOk) {
        Serial.println("   Cannot recover - connection check failed");
        return false;
    }
    
    // Check that the conveyor is not in ALARM
    if (_conveyor->getState() == "ALARM") {
        Serial.println("   Cannot recover - conveyor still in ALARM state");
        return false;
    }
    
    Serial.println("   All recovery conditions met");
    return true;
}