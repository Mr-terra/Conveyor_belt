#include "scheduler.h"
#include <Arduino.h>

// State constants for scheduler
const String STATE_WAITING = "WAITING";
const String STATE_CHECK_CONNECT = "CHECK_CONNECT";
const String STATE_INIT_POSITIONS = "INIT_POSITIONS";
const String STATE_PRODUCTION = "PRODUCTION";

// Global variable for scheduler state
String schedulerState = STATE_WAITING;

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