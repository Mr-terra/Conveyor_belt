#include "servo_sorter.h"
#include <M5Stack.h>
#include "GoPlus2.h"

static GoPlus2 goPlus;

void ServoSorter::init()
{
    goPlus.begin();
    goPlus.Servo_write_angle(SERVO_NUM0, 90); // neutre
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
        return;
    goPlus.Servo_write_angle(SERVO_NUM0, angleFor(bin));
}
