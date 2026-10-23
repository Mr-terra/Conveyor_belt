// ServoSorter: the diverter servo, driven through the GoPlus2 board.

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

// Drives the diverter. moveTo() turns the servo to the angle of a bin.
// A parcel without a bin goes to the error bin.
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

    // Servo angle in degrees for each bin: BIN1 = 0, BIN2 = 30, BIN3 = 60.
    int angleFor(Bin::Id bin) const;

    // Observable internal state
    ServoState servoState = ServoState::Init;

    // Helper for StateManager
    static String stateToString(ServoState s);
};
