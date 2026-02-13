#pragma once
#include <Arduino.h>

class RFIDReader
{
public:
    void init();
    String readUid();
    
    // Health check - vérifie la communication I2C sans init complète
    static bool checkConnection();
};
