#pragma once
#include <Arduino.h>

class RFIDReader
{
public:
    void init();
    String readUid();
};
