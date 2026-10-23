#pragma once
#include <Arduino.h>
#include "state_manager.h"
#include "hal/i2c_bus.h"
#include "hal/rfid_driver.h"
#include "hal/servo_driver.h"

class SorterController
{
public:
    // Injects the hardware drivers. Must be called before initialize().
    void setHardware(IRfidDriver *rfidDriver, IServoDriver *servoDriver, II2CBus *bus);

    void initialize(StateManager &sm);
    void production();

    // Health check - verifies the whole sorting system
    bool checkConnection();

    void setState(const char *newState);

private:
    StateManager *sm = nullptr;
    String sorterState = "INIT_PRODUCTION";

    static String stateToString(String s);
    static void setupStateRules(StateManager &stateManager);
};

extern SorterController sorterController;
