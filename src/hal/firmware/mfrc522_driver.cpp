// Adapter: the MFRC522 RFID reader (I2C) behind the IRfidDriver interface.
#include "hal/firmware/adapters.h"

#include <MFRC522_I2C.h>
#include <Wire.h>

namespace
{
    // I2C address and reset pin of the RFID reader
    constexpr uint8_t RFID_I2C_ADDRESS = 0x28;
    constexpr uint8_t RFID_RESET_PIN = 33;

    class Mfrc522Driver : public IRfidDriver
    {
    public:
        Mfrc522Driver() : _mfrc522(RFID_I2C_ADDRESS, RFID_RESET_PIN) {}

        void init() override
        {
            Wire.begin(21, 22);
            _mfrc522.PCD_Init();
        }
        bool isNewCardPresent() override { return _mfrc522.PICC_IsNewCardPresent(); }
        bool readCardSerial() override { return _mfrc522.PICC_ReadCardSerial(); }

        uint8_t uidBytes(uint8_t *buffer, uint8_t maxLength) override
        {
            uint8_t size = _mfrc522.uid.size;
            if (size > maxLength)
                size = maxLength;
            for (uint8_t i = 0; i < size; i++)
                buffer[i] = _mfrc522.uid.uidByte[i];
            return size;
        }

        void halt() override
        {
            _mfrc522.PICC_HaltA();
            _mfrc522.PCD_StopCrypto1();
        }

    private:
        MFRC522_I2C _mfrc522;
    };
}

IRfidDriver *rfidAdapter()
{
    static Mfrc522Driver driver;
    return &driver;
}
