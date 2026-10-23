// Firmware-only adapters: the only place that talks to the M5Stack / ESP32 libraries.
// The logic in src/ depends on the interfaces in include/hal/ and never on these.
#include "hal/firmware_hal.h"

#include <M5Stack.h>
#include <HTTPClient.h>
#include <MFRC522_I2C.h>
#include <Module_GRBL_13.2.h>
#include <WiFi.h>
#include <Wire.h>
#include "GoPlus2.h"

#include "conveyor.h"
#include "globals.h"
#include "main_sorter.h"

extern Conveyor conveyor;

namespace
{
    constexpr uint8_t CONVEYOR_I2C_ADDRESS = 0x70;
    constexpr uint8_t RFID_I2C_ADDRESS = 0x28;
    constexpr uint8_t RFID_RESET_PIN = 33;

    class GrblModuleMotor : public IGrblMotor
    {
    public:
        explicit GrblModuleMotor(uint8_t address) : _grbl(address) {}

        void init() override { _grbl.Init(&Wire); }
        void setMode(const char *mode) override { _grbl.setMode(mode); }
        void readClean() override { _grbl.readClean(); }
        bool inLock() override { return _grbl.inLock(); }
        void unLock() override { _grbl.unLock(); }
        void setMotor(int a, int b, int c, int d) override { _grbl.setMotor(a, b, c, d); }
        void waitIdle() override { _grbl.waitIdle(); }
        String readStatus() override { return _grbl.readStatus(); }
        bool readIdle() override { return _grbl.readIdle(); }

    private:
        Module_GRBL _grbl;
    };

    class WireI2CBus : public II2CBus
    {
    public:
        uint8_t probe(uint8_t address) override
        {
            Wire.begin();
            Wire.beginTransmission(address);
            return Wire.endTransmission();
        }
    };

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

    class GoPlusServoDriver : public IServoDriver
    {
    public:
        void begin() override { _goPlus.begin(); }
        void writeAngle(uint8_t channel, int angle) override { _goPlus.Servo_write_angle(channel, angle); }

    private:
        GoPlus2 _goPlus;
    };

    class HttpClientTransport : public IHttpClient
    {
    public:
        HttpResponse get(const String &url, const String &apiKey) override
        {
            HTTPClient http;
            begin(http, url);
            http.addHeader("DOLAPIKEY", apiKey);
            return perform(http, http.GET());
        }

        HttpResponse post(const String &url, const String &apiKey, const String &body) override
        {
            HTTPClient http;
            begin(http, url);
            http.addHeader("Content-Type", "application/json");
            http.addHeader("DOLAPIKEY", apiKey);
            return perform(http, http.POST(body));
        }

    private:
        // The loop is blocked during a request, so a silent server must not stall it for long
        static constexpr uint16_t TIMEOUT_MS = 5000;

        static void begin(HTTPClient &http, const String &url)
        {
            http.begin(url);
            http.setTimeout(TIMEOUT_MS);
        }

        static HttpResponse perform(HTTPClient &http, int code)
        {
            HttpResponse response;
            response.statusCode = code;
            if (code > 0)
                response.body = http.getString();
            http.end();
            return response;
        }
    };

    class WifiNetwork : public INetwork
    {
    public:
        bool isConnected() override { return WiFi.status() == WL_CONNECTED; }
        String localIp() override { return WiFi.localIP().toString(); }
        String gatewayIp() override { return WiFi.gatewayIP().toString(); }
    };

    GrblModuleMotor conveyorMotor(CONVEYOR_I2C_ADDRESS);
    WireI2CBus wireBus;
    Mfrc522Driver rfidDriver;
    GoPlusServoDriver servoDriver;
    HttpClientTransport httpClient;
    WifiNetwork network;
}

void bindFirmwareHardware()
{
    conveyor.setHardware(&conveyorMotor, &wireBus);
    sorterController.setHardware(&rfidDriver, &servoDriver, &wireBus);
    dolibarrApi.setTransport(&httpClient, &network);
}
