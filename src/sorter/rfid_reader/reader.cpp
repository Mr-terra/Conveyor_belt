#include "reader.h"

#define RFID_ADDR 0x28

static const uint32_t COOLDOWN_MS = 1500;

void RFIDReader::setHardware(IRfidDriver *driver, II2CBus *bus)
{
    _driver = driver;
    _bus = bus;
    _hasTriggered = false;
}

void RFIDReader::init()
{
    _driver->init();
}

String RFIDReader::readUid()
{
    if (!_driver->isNewCardPresent())
        return "";
    if (!_driver->readCardSerial())
        return "";

    uint32_t now = millis();
    if (_hasTriggered && now - _lastTriggerMs < COOLDOWN_MS)
    {
        _driver->halt();
        return "";
    }
    _hasTriggered = true;
    _lastTriggerMs = now;

    uint8_t uidBytes[10];
    uint8_t size = _driver->uidBytes(uidBytes, sizeof(uidBytes));

    String uid;
    for (uint8_t i = 0; i < size; i++)
    {
        if (uidBytes[i] < 0x10)
            uid += "0";
        uid += String(uidBytes[i], HEX);
        if (i < size - 1)
            uid += ":";
    }
    uid.toUpperCase();

    _driver->halt();
    return uid;
}

bool RFIDReader::checkConnection()
{
    // Simple check: probe the I2C address of the RFID module
    uint8_t error = _bus->probe(RFID_ADDR);

    if (error == 0) {
        Serial.println("✅ RFID I2C detected at 0x28");
        return true;
    } else {
        Serial.printf("❌ RFID not detected at 0x28 (error: %d)\n", error);
        return false;
    }
}
