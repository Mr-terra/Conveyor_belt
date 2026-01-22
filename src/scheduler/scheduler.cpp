#include "scheduler.h"
#include <Arduino.h>

// State constants for scheduler (now declared in header)
const String STATE_WAITING = "WAITING";
const String STATE_CHECK_CONNECT = "CHECK_CONNECT";
const String STATE_INIT_POSITIONS = "INIT_POSITIONS";
const String STATE_PRODUCTION = "PRODUCTION";

// State conversion function for automatic monitoring
String Scheduler::stateToString(String state) {
    if (state == STATE_WAITING) return "WAITING";
    if (state == STATE_CHECK_CONNECT) return "CHECK_CONNECT";
    if (state == STATE_INIT_POSITIONS) return "INIT_POSITIONS";
    if (state == STATE_PRODUCTION) return "PRODUCTION";
    return "UNKNOWN";
}

// External state variable (defined in main.cpp)
extern String schedulerState;

void Scheduler::initialize() {
    schedulerState = STATE_WAITING;
    Serial.println("Scheduler initialized. State: WAITING");
}

void Scheduler::setState(String state) {
    schedulerState = state;
    Serial.print("Scheduler state changed to: ");
    Serial.println(state);
}

String Scheduler::getState() {
    return schedulerState;
}

void Scheduler::triggerCheckConnect() {
    setState(STATE_CHECK_CONNECT);
}

void Scheduler::triggerInitPositions() {
    setState(STATE_INIT_POSITIONS);
}

void Scheduler::triggerProduction() {
    setState(STATE_PRODUCTION);
}

void Scheduler::triggerStop() {
    setState(STATE_WAITING);
}