#pragma once
#include <gmock/gmock.h>

class MockWire {
public:
    MOCK_METHOD(uint8_t, beginTransmission, (uint8_t), ());
    MOCK_METHOD(uint8_t, endTransmission, (), ());
};