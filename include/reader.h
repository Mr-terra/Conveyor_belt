#pragma once
#include <Arduino.h>
#include "hal/i2c_bus.h"
#include "hal/rfid_driver.h"

class RFIDReader
{
public:
    // Injects the hardware drivers. Must be called before init().
    void setHardware(IRfidDriver *driver, II2CBus *bus);

    void init();
    String readUid();

    // Health check - verifies I2C communication without a full init
    bool checkConnection();

private:
    IRfidDriver *_driver = nullptr;
    II2CBus *_bus = nullptr;
    bool _hasTriggered = false;
    uint32_t _lastTriggerMs = 0;
};
