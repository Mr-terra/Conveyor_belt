#include "servo_sorter.h"
#include <M5Stack.h>
#include "GoPlus2.h"

static GoPlus2 goPlus;

int init_angle = 30;

String ServoSorter::stateToString(String s)
{
    return s;
}

void ServoSorter::init(StateManager &stateManager)
{
    goPlus.begin();
    goPlus.Servo_write_angle(SERVO_NUM0, init_angle); // neutre

    // Observe the servo state
    stateManager.observeState<String>(
        &servoState,
        "SERVO",
        stateToString);

    // Rules (static, no dynamic rules here)
    stateManager.addStateRule("SERVO", "INIT", "*", ERROR_INFO, "Servo initialized");
    stateManager.addStateRule("SERVO", "*", "ERROR", ERROR_CRITICAL, "Servo error");

    servoState = "IDLE";
}

int ServoSorter::angleFor(Bin::Id bin) const
{
    Serial.print("angleFor() bin=");
    Serial.println((int)bin + "\n");

    switch (bin)
    {
    case Bin::BIN1:
        Serial.print("BIN1 angle");
        return 0;
    case Bin::BIN2:
        Serial.print("BIN2 angle");

        return 30;
    case Bin::BIN3:
        Serial.print("BIN3 angle");
        return 60;
    default:
        Serial.print("Default angle");
        return 0;
    }
}

void ServoSorter::moveTo(Bin::Id bin)
{
    Serial.print("Servo called with bin: ");
    Serial.println((int)bin);

    if (bin == Bin::NONE)
    {
        servoState = "ERROR";
    }

    servoState = "MOVING";

    uint8_t angle = (uint8_t)angleFor(bin);

    Serial.print("Servo moved to angle: ");
    Serial.println(angle);

    goPlus.Servo_write_angle(SERVO_NUM0, angle);

    servoState = "IDLE";
}

bool ServoSorter::checkConnection()
{
    Wire.beginTransmission(GOPLUS_ADDR);
    byte error = Wire.endTransmission();

    if (error == 0)
    {
        Serial.println("✅ Servo/GoPlus2 I2C detected at 0x38");
        return true;
    }
    else
    {
        Serial.printf("❌ Servo/GoPlus2 not detected at 0x38 (error: %d)\n", error);
        return false;
    }
}
