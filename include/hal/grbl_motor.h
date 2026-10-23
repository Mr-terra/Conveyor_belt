#pragma once
#include <Arduino.h>

// Hardware abstraction for the GRBL 13.2 motor module (Module_GRBL).
class IGrblMotor
{
public:
    virtual ~IGrblMotor() = default;

    virtual void init() = 0;
    virtual void setMode(const char *mode) = 0;
    virtual void readClean() = 0;
    virtual bool inLock() = 0;
    virtual void unLock() = 0;
    virtual void setMotor(int a, int b, int c, int d) = 0;
    virtual void waitIdle() = 0;
    virtual String readStatus() = 0;
    virtual bool readIdle() = 0;
};
