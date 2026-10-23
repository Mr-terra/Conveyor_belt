// gMock doubles for every hardware abstraction in include/hal/.
#pragma once

#include <gmock/gmock.h>

#include "hal/grbl_motor.h"
#include "hal/http_client.h"
#include "hal/i2c_bus.h"
#include "hal/network.h"
#include "hal/rfid_driver.h"
#include "hal/servo_driver.h"

class MockGrblMotor : public IGrblMotor
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, setMode, (const char *), (override));
    MOCK_METHOD(void, readClean, (), (override));
    MOCK_METHOD(bool, inLock, (), (override));
    MOCK_METHOD(void, unLock, (), (override));
    MOCK_METHOD(void, setMotor, (int, int, int, int), (override));
    MOCK_METHOD(void, waitIdle, (), (override));
    MOCK_METHOD(String, readStatus, (), (override));
    MOCK_METHOD(bool, readIdle, (), (override));
};

class MockI2CBus : public II2CBus
{
public:
    MOCK_METHOD(uint8_t, probe, (uint8_t), (override));
};

class MockRfidDriver : public IRfidDriver
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(bool, isNewCardPresent, (), (override));
    MOCK_METHOD(bool, readCardSerial, (), (override));
    MOCK_METHOD(uint8_t, uidBytes, (uint8_t *, uint8_t), (override));
    MOCK_METHOD(void, halt, (), (override));
};

class MockServoDriver : public IServoDriver
{
public:
    MOCK_METHOD(void, begin, (), (override));
    MOCK_METHOD(void, writeAngle, (uint8_t, int), (override));
};

class MockHttpClient : public IHttpClient
{
public:
    MOCK_METHOD(HttpResponse, get, (const String &, const String &), (override));
    MOCK_METHOD(HttpResponse, post, (const String &, const String &, const String &), (override));
};

class MockNetwork : public INetwork
{
public:
    MOCK_METHOD(bool, isConnected, (), (override));
    MOCK_METHOD(String, localIp, (), (override));
    MOCK_METHOD(String, gatewayIp, (), (override));
};
