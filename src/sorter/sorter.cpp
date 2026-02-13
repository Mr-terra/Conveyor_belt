#include "sorter.h"
#include <Arduino.h>

Sorter sorter;

// Constants for sorter pin
#define SORTER_PIN 6

// State constants for sorter (now declared in header)
const String SORTER_STATE_INITIAL = "INITIAL";
const String SORTER_STATE_MOVING = "MOVING";
const String SORTER_STATE_ERROR = "ERROR";

// State conversion function for automatic monitoring
String Sorter::stateToString(String state) {
    if (state == SORTER_STATE_INITIAL) return "INITIAL";
    if (state == SORTER_STATE_MOVING) return "MOVING";
    if (state == SORTER_STATE_ERROR) return "ERROR";
    return "UNKNOWN";
}

void Sorter::initialize() {
    // Initialize sorter pin
    pinMode(SORTER_PIN, OUTPUT);
    
    _current_state = SORTER_STATE_INITIAL;
    digitalWrite(SORTER_PIN, LOW);
    Serial.println("Sorter initialized. State: INITIAL");
}

void Sorter::moveTo(String direction) {
    _current_state = SORTER_STATE_MOVING;
    // Simulation of movement
    Serial.print("Moving sorter to ");
    Serial.println(direction);
    // Here you would add the actual code to move the sorter to the specified direction
}

void Sorter::stop() {
    _current_state = SORTER_STATE_INITIAL;
    digitalWrite(SORTER_PIN, LOW);
    Serial.println("Sorter stopped");
}

String Sorter::getState() {
    return _current_state;
}

void Sorter::setState(String state) {
    _current_state = state;
    if (state == SORTER_STATE_MOVING) {
        // Here you would add the actual code to start moving the sorter
    } else {
        digitalWrite(SORTER_PIN, LOW);
    }
    Serial.print("Sorter state changed to: ");
    Serial.println(state);
}