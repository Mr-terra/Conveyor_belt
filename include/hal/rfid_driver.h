#pragma once
#include <Arduino.h>

// Hardware abstraction for the MFRC522 RFID reader (I2C).
class IRfidDriver
{
public:
    virtual ~IRfidDriver() = default;

    virtual void init() = 0;
    virtual bool isNewCardPresent() = 0;
    virtual bool readCardSerial() = 0;
    // Copies the UID of the last read card into `buffer` and returns its size.
    virtual uint8_t uidBytes(uint8_t *buffer, uint8_t maxLength) = 0;
    // Halts the card and stops the crypto unit.
    virtual void halt() = 0;
};
