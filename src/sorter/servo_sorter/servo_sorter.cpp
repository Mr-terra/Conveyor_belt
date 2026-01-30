#include "servo_sorter.h"
#include <M5Stack.h>
#include "GoPlus2.h"

static GoPlus2 goPlus;

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
    switch (bin)
    {
    case Bin::BIN1:
        return 0;
    case Bin::BIN2:
        return 90;
    case Bin::BIN3:
        return 180;
    default:
        return 90;
    }
}

void ServoSorter::moveTo(Bin::Id bin)
{
    if (bin == Bin::NONE)
    {
        servoState = "ERROR";
        return;
    }

    servoState = "MOVING";

    int angle = angleFor(bin);
    goPlus.Servo_write_angle(SERVO_NUM0, angle);

    // ici tu pourrais plus tard vérifier un retour matériel
    servoState = "IDLE";
}
