// #include <Arduino.h>
// #include <Wire.h>

// #include <M5Stack.h>
// #include "GoPlus2.h"

// #include <MFRC522_I2C.h>

// // ---------------- RFID ----------------
// #define RFID_ADDR 0x28
// #define RST_PIN 33
// MFRC522_I2C mfrc522(RFID_ADDR, RST_PIN);

// // ---------------- GOPlus2 / Servo ----------------
// GoPlus2 goPlus;

// // On GoPlus2: SERVO_NUM0..3 usually map to S1..S4
// #define SERVO_CH SERVO_NUM0
// #define SERVO_0 0
// #define SERVO_90 90

// static uint32_t lastTriggerMs = 0;
// static const uint32_t COOLDOWN_MS = 1500;

// static void printUid()
// {
//     Serial.print("UID: ");
//     for (byte i = 0; i < mfrc522.uid.size; i++)
//     {
//         if (mfrc522.uid.uidByte[i] < 0x10)
//             Serial.print("0");
//         Serial.print(mfrc522.uid.uidByte[i], HEX);
//         if (i < mfrc522.uid.size - 1)
//             Serial.print(":");
//     }
//     Serial.println();
// }

// static void rotateServo90Once()
// {
//     goPlus.Servo_write_angle(SERVO_CH, SERVO_90);
//     delay(500);
//     goPlus.Servo_write_angle(SERVO_CH, SERVO_0);
//     delay(300);
// }

// void setup()
// {
//     M5.begin();
//     Serial.begin(115200);
//     delay(800);

//     // I2C Core Basic : SDA=21, SCL=22
//     Wire.begin(21, 22);

//     // Init GoPlus2 (utilise I2C)
//     goPlus.begin();
//     goPlus.Servo_write_angle(SERVO_CH, SERVO_0);

//     // Init RFID
//     mfrc522.PCD_Init();

//     Serial.println("Init OK (RFID + GOPlus2). Bring a card close...");
// }

// void loop()
// {
//     M5.update();

//     if (!mfrc522.PICC_IsNewCardPresent())
//     {
//         delay(20);
//         return;
//     }
//     if (!mfrc522.PICC_ReadCardSerial())
//     {
//         delay(20);
//         return;
//     }
//     uint32_t now = millis();
//     if (now - lastTriggerMs < COOLDOWN_MS)
//     {
//         mfrc522.PICC_HaltA();
//         mfrc522.PCD_StopCrypto1();
//         delay(50);
//         return;
//     }
//     lastTriggerMs = now;

//     printUid();
//     rotateServo90Once();

//     mfrc522.PICC_HaltA();
//     mfrc522.PCD_StopCrypto1();
//     delay(50);
// }
