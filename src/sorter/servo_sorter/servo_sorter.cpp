#include "servo_sorter.h"
#include <M5Stack.h>
#include "GoPlus2.h"

static GoPlus2 goPlus;

int init_angle = 90;

String ServoSorter::stateToString(String s)
{
    return s;
}

void ServoSorter::init(StateManager &stateManager)
{
    goPlus.begin();
    goPlus.Servo_write_angle(SERVO_NUM0, 90); // neutre

    // Observer l’état du servo
    stateManager.observeState<String>(
        &servoState,
        "SERVO",
        stateToString);

    // Règles (statique, pas de dynamique ici)
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
        Serial.print("Wazup bro, it's BIN1\n");
        return 0;
    case Bin::BIN2:
        Serial.print("Wazup bro, it's BIN2\n");
        return 90;
    case Bin::BIN3:
        Serial.print("Wazup bro, it's BIN3\n");
        return 180;
    default:
        Serial.print("WTF is that ?\n");
        return 90;
    }
}

void ServoSorter::moveTo(Bin::Id bin)
{
    Serial.print("Servo called with bin: ");
    Serial.println((int)bin);

    if (bin == Bin::NONE)
    {
        servoState = "ERROR";
        return;
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
    // Vérification : tester si le module GoPlus2 répond sur I2C
    Wire.beginTransmission(GOPLUS_ADDR); // Adresse du GoPlus2 (0x38)
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
