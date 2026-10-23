// Adapter: the GRBL 13.2 motor module (Module_GRBL) behind the IGrblMotor interface.
#include "hal/firmware/adapters.h"

#include <Module_GRBL_13.2.h>
#include <Wire.h>

namespace
{
    // I2C address of the GRBL module
    constexpr uint8_t CONVEYOR_I2C_ADDRESS = 0x70;

    class GrblModuleMotor : public IGrblMotor
    {
    public:
        explicit GrblModuleMotor(uint8_t address) : _grbl(address) {}

        void init() override { _grbl.Init(&Wire); }
        void setMode(const char *mode) override { _grbl.setMode(mode); }
        void readClean() override { _grbl.readClean(); }
        bool inLock() override { return _grbl.inLock(); }
        void unLock() override { _grbl.unLock(); }
        void setMotor(int a, int b, int c, int d) override { _grbl.setMotor(a, b, c, d); }
        void waitIdle() override { _grbl.waitIdle(); }
        String readStatus() override { return _grbl.readStatus(); }
        bool readIdle() override { return _grbl.readIdle(); }

    private:
        Module_GRBL _grbl;
    };
}

IGrblMotor *grblMotorAdapter()
{
    static GrblModuleMotor motor(CONVEYOR_I2C_ADDRESS);
    return &motor;
}
