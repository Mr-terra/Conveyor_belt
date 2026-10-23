#pragma once
#include <Arduino.h>

// Hardware abstraction for the servo driver board (GoPlus2).
class IServoDriver
{
public:
    virtual ~IServoDriver() = default;

    virtual void begin() = 0;
    virtual void writeAngle(uint8_t channel, int angle) = 0;
};
