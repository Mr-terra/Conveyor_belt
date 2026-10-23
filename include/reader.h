// RFIDReader: reads card UIDs, with a cooldown so the same tag is not read twice.

#pragma once
#include <Arduino.h>
#include "hal/i2c_bus.h"
#include "hal/rfid_driver.h"

// Wraps the RFID driver. The same card is accepted at most once per COOLDOWN_MS (1.5 s).
class RFIDReader
{
public:
    // Injects the hardware drivers. Must be called before init().
    void setHardware(IRfidDriver *driver, II2CBus *bus);

    // Starts the reader chip.
    void init();
    // Returns the UID as hex pairs separated by ':' (for example 04:1A:2B:3C).
    // Returns an empty string when no card is read, or when the card is inside the cooldown.
    String readUid();

    // Health check - verifies I2C communication without a full init
    bool checkConnection();

private:
    IRfidDriver *_driver = nullptr;
    II2CBus *_bus = nullptr;
    // True after the first accepted read.
    bool _hasTriggered = false;
    // Time of the last accepted read, in milliseconds (millis()).
    uint32_t _lastTriggerMs = 0;
};
