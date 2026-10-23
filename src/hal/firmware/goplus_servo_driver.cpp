// Adapter: the GoPlus2 servo board (I2C) behind the IServoDriver interface.
#include "hal/firmware/adapters.h"

#include "GoPlus2.h"

namespace
{
    class GoPlusServoDriver : public IServoDriver
    {
    public:
        void begin() override { _goPlus.begin(); }
        void writeAngle(uint8_t channel, int angle) override { _goPlus.Servo_write_angle(channel, angle); }

    private:
        GoPlus2 _goPlus;
    };
}

IServoDriver *servoAdapter()
{
    static GoPlusServoDriver driver;
    return &driver;
}
