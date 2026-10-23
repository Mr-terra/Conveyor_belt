#pragma once
#include <Arduino.h>
#include "state_manager.h"
#include "hal/i2c_bus.h"
#include "hal/rfid_driver.h"
#include "hal/servo_driver.h"

// States of one sorting cycle. SorterController::stateToString() gives the name used by the rules and logs.
enum class SorterState : uint8_t
{
    InitProduction,
    Waiting,
    RfidRead,
    RfidReadingError
};

class SorterController
{
public:
    // Injects the hardware drivers. Must be called before initialize().
    void setHardware(IRfidDriver *rfidDriver, IServoDriver *servoDriver, II2CBus *bus);

    void initialize(StateManager &sm);
    void production();

    // Health check - verifies the whole sorting system
    bool checkConnection();

    void setState(SorterState newState);

private:
    StateManager *sm = nullptr;
    SorterState sorterState = SorterState::InitProduction;

    static String stateToString(SorterState s);
    static void setupStateRules(StateManager &stateManager);
};

extern SorterController sorterController;
