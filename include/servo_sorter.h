#pragma once
#include <Arduino.h>
#include "hal/i2c_bus.h"
#include "hal/servo_driver.h"
#include "sorter_types.h"
#include "state_manager.h"

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
    String servoState = "INIT";

    // Helper for StateManager
    static String stateToString(String s);
};
