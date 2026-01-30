#pragma once
#include <Arduino.h>

class Bin
{
public:
    enum Id : uint8_t
    {
        NONE = 0,
        BIN1 = 1,
        BIN2 = 2,
        BIN3 = 3
    };

    static const char *toString(Id id)
    {
        switch (id)
        {
        case BIN1:
            return "BIN1";
        case BIN2:
            return "BIN2";
        case BIN3:
            return "BIN3";
        default:
            return "NONE";
        }
    }

private:
    Bin() = delete; // classe utilitaire uniquement
};
