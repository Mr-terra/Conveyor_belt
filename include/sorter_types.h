// Types shared by the sorting modules.

#pragma once
#include <Arduino.h>

// The sorter bins. NONE means no bin: the card could not be routed.
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

    // Parcels that cannot be routed (unknown card, failed lookup, failed stock
    // movement) go to this bin. It is the same physical bin as BIN1.
    static constexpr Id ERROR_BIN = BIN1;

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
