#pragma once
#include <gmock/gmock.h>
#include <String.h>

class MockGRBL {
public:
    MOCK_METHOD(String, readStatus, (), ());
    MOCK_METHOD(bool, inLock, (), ());
    MOCK_METHOD(bool, readIdle, (), ());
    MOCK_METHOD(void, unLock, (), ());
    MOCK_METHOD(void, setMotor, (int, int, int, int), ());
    MOCK_METHOD(void, waitIdle, (), ());
};