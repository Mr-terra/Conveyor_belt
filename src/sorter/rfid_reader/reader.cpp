// RFIDReader: reads card UIDs, with a cooldown so the same card is not read twice.

#include "reader.h"
#include "log.h"

// I2C address of the MFRC522 reader.
#define RFID_ADDR 0x28

// A card seen again within this time is ignored: it is usually the same tag still on the reader.
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

    // A UID has at most 10 bytes.
    uint8_t uidBytes[10];
    uint8_t size = _driver->uidBytes(uidBytes, sizeof(uidBytes));

    String uid;
    // Each byte becomes two hex digits. A ':' goes between bytes, for example 04:1A:2B.
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
        LOG_INFO("RFID", "reader detected at 0x28");
        return true;
    } else {
        LOG_ERROR("RFID", "reader not detected at 0x28 (I2C error %d)", error);
        return false;
    }
}
