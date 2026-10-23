#include "scheduler.h"
#include "main_sorter.h"
#include "check_connect.h"
#include "init_position.h"
#include "conveyor.h"
#include <Arduino.h>

Scheduler scheduler;

// State constants for scheduler (now declared in header)
const String STATE_WAITING = "WAITING";
const String STATE_CHECK_CONNECT = "CHECK_CONNECT";
const String STATE_INIT_POSITIONS = "INIT_POSITIONS";
const String STATE_PRODUCTION = "PRODUCTION";
const String STATE_ERROR = "ERROR";

// State conversion function for automatic monitoring
String Scheduler::stateToString(String state)
{
    if (state == STATE_WAITING)
        return "WAITING";
    if (state == STATE_CHECK_CONNECT)
        return "CHECK_CONNECT";
    if (state == STATE_INIT_POSITIONS)
        return "INIT_POSITIONS";
    if (state == STATE_PRODUCTION)
        return "PRODUCTION";
    if (state == STATE_ERROR)
        return "ERROR";
    return "UNKNOWN";
}

void Scheduler::initialize() {
    _current_state = STATE_WAITING;
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

void Scheduler::setState(String state) {
    _current_state = state;
    Serial.print("Scheduler state changed to: ");
    Serial.println(state);
}

String Scheduler::getState() {
    return _current_state;
}

void Scheduler::triggerCheckConnect()
{
    if (_checkConnect == nullptr) {
        Serial.println("❌ Cannot trigger CHECK_CONNECT: CheckConnect not configured");
        setState(STATE_ERROR);
        return;
    }
    setState(STATE_CHECK_CONNECT);
}

void Scheduler::triggerInitPositions()
{
    if (_initPosition == nullptr) {
        Serial.println("❌ Cannot trigger INIT_POSITIONS: InitPosition not configured");
        setState(STATE_ERROR);
        return;
    }
    setState(STATE_INIT_POSITIONS);
}

void Scheduler::triggerProduction()
{
    if (_sorterController == nullptr || _conveyor == nullptr) {
        Serial.println("❌ Cannot trigger PRODUCTION: Dependencies not configured");
        setState(STATE_ERROR);
        return;
    }
    setState(STATE_PRODUCTION);
}

void Scheduler::triggerStop()
{
    setState(STATE_WAITING);
    if (_conveyor != nullptr) {
        _conveyor->stop();
    }
}

// Main method that runs the logic of the current state
void Scheduler::executeCurrentState() {
    // ERROR state is handled first
    if (_current_state == STATE_ERROR) {
        Serial.println("⚠️  System in ERROR state - checking recovery conditions");
        
        // Check whether we can leave the ERROR state
        if (canRecoverFromError()) {
            Serial.println("🔧 Error conditions resolved - returning to WAITING");
            Error::clearCriticalErrors();
            setState(STATE_WAITING);
        } else {
            Serial.println("🚫 Still in ERROR state - manual intervention may be required");
        }
        return; // Do not run other states in ERROR mode
    }
    
    if (_current_state == STATE_CHECK_CONNECT) {
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
    else if (_current_state == STATE_INIT_POSITIONS) {
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
    else if (_current_state == STATE_PRODUCTION) {
        // Check the conveyor state before continuing
        if (_conveyor != nullptr && _conveyor->getState() == "ALARM") {
            Serial.println("❌ Conveyor in ALARM state during production - stopping");
            setState(STATE_WAITING);
            return;
        }
        
        if (_conveyor != nullptr) {
            _conveyor->move(true); // Mouvement continu
        }
        if (_sorterController != nullptr) {
            _sorterController->production();
        }
    }
    else if (_current_state == STATE_WAITING) {
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