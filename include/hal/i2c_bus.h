#pragma once
#include <Arduino.h>

// Hardware abstraction for the I2C bus.
class II2CBus
{
public:
    virtual ~II2CBus() = default;

    // Sends an empty transmission to `address`.
    // Returns 0 when the device acknowledges (same convention as Wire.endTransmission()).
    virtual uint8_t probe(uint8_t address) = 0;
};
