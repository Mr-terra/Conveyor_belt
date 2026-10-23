#pragma once
#include <Arduino.h>

class RFIDReader
{
public:
    void init();
    String readUid();
    
    // Health check - verifies I2C communication without a full init
    static bool checkConnection();
};
