#pragma once
#include <Arduino.h>
#include "hal/i2c_bus.h"
#include "hal/servo_driver.h"
#include "sorter_types.h"
#include "state_manager.h"

// States of the diverter servo. ServoSorter::stateToString() gives the name used by the rules and logs.
enum class ServoState : uint8_t
{
    Init,
    Idle,
    Moving
};

class ServoSorter
{
public:
    // Injects the hardware drivers. Must be called before init().
    void setHardware(IServoDriver *driver, II2CBus *bus);

    void init(StateManager &stateManager);
    void moveTo(Bin::Id bin);

    // Health check - verifies that the servo driver responds
    bool checkConnection();

private:
    IServoDriver *_driver = nullptr;
    II2CBus *_bus = nullptr;

    int angleFor(Bin::Id bin) const;

    // Observable internal state
    ServoState servoState = ServoState::Init;

    // Helper for StateManager
    static String stateToString(ServoState s);
};
