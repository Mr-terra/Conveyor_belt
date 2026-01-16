#include "conveyor.h"
#include <Arduino.h>

// Constants for conveyor motor pin
#define CONVEYOR_MOTOR_PIN 5

// State constants for conveyor
const String CONVEYOR_STATE_STOPPED = "STOPPED";
const String CONVEYOR_STATE_RUNNING = "RUNNING";
const String CONVEYOR_STATE_ERROR = "ERROR";

// Global variable for conveyor state
String conveyorState = CONVEYOR_STATE_STOPPED;

void setup() {
    // Initialize conveyor motor pin
    pinMode(CONVEYOR_MOTOR_PIN, OUTPUT);
    
    // Initialize serial communication
    Serial.begin(9600);
    Serial.println("Conveyor initialization...");
    
    // Set initial state
    conveyorState = CONVEYOR_STATE_STOPPED;
    digitalWrite(CONVEYOR_MOTOR_PIN, LOW);
    Serial.println("Conveyor initialized. State: STOPPED");
}

void loop() {
    // Conveyor state management
    if (conveyorState == CONVEYOR_STATE_RUNNING) {
        // Keep motor running
        digitalWrite(CONVEYOR_MOTOR_PIN, HIGH);
    } else {
        // Keep motor stopped
        digitalWrite(CONVEYOR_MOTOR_PIN, LOW);
    }
    
    // Delay to avoid overload
    delay(100);
}

// Conveyor functions

void Conveyor::initialize() {
    conveyorState = CONVEYOR_STATE_STOPPED;
    digitalWrite(CONVEYOR_MOTOR_PIN, LOW);
}

void Conveyor::start() {
    conveyorState = CONVEYOR_STATE_RUNNING;
    digitalWrite(CONVEYOR_MOTOR_PIN, HIGH);
    Serial.println("Conveyor started");
}

void Conveyor::stop() {
    conveyorState = CONVEYOR_STATE_STOPPED;
    digitalWrite(CONVEYOR_MOTOR_PIN, LOW);
    Serial.println("Conveyor stopped");
}

String Conveyor::getState() {
    return conveyorState;
}

void Conveyor::setState(String state) {
    conveyorState = state;
    if (state == CONVEYOR_STATE_RUNNING) {
        digitalWrite(CONVEYOR_MOTOR_PIN, HIGH);
    } else {
        digitalWrite(CONVEYOR_MOTOR_PIN, LOW);
    }
    Serial.print("Conveyor state changed to: ");
    Serial.println(state);
}