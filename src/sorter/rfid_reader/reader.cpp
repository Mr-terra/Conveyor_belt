#include "reader.h"
#include <Wire.h>
#include <MFRC522_I2C.h>

#define RFID_ADDR 0x28
#define RST_PIN 33

static MFRC522_I2C mfrc522(RFID_ADDR, RST_PIN);
static uint32_t lastTriggerMs = 0;
static const uint32_t COOLDOWN_MS = 1500;

void RFIDReader::init()
{
    Wire.begin(21, 22);
    mfrc522.PCD_Init();
}

String RFIDReader::readUid()
{
    if (!mfrc522.PICC_IsNewCardPresent())
        return "";
    if (!mfrc522.PICC_ReadCardSerial())
        return "";

    uint32_t now = millis();
    if (now - lastTriggerMs < COOLDOWN_MS)
    {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return "";
    }
    lastTriggerMs = now;

    String uid;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        if (mfrc522.uid.uidByte[i] < 0x10)
            uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1)
            uid += ":";
    }
    uid.toUpperCase();

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return uid;
}
