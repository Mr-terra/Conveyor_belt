// Adapter: the ESP32 I2C bus (Wire) behind the II2CBus interface.
#include "hal/firmware/adapters.h"

#include <Wire.h>

namespace
{
    class WireI2CBus : public II2CBus
    {
    public:
        // probe() starts a transmission to the address; the result is 0 when the device acknowledges.
        uint8_t probe(uint8_t address) override
        {
            Wire.begin();
            Wire.beginTransmission(address);
            return Wire.endTransmission();
        }
    };
}

II2CBus *wireBusAdapter()
{
    static WireI2CBus bus;
    return &bus;
}
