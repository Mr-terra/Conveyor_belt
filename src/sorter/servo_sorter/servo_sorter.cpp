// ServoSorter: drives the diverter servo through the GoPlus2 board (see servo_sorter.h).

#include "servo_sorter.h"
#include "log.h"

// Servo channel on the GoPlus2 board and I2C address of the board
static const uint8_t SERVO_CHANNEL = 0;
static const uint8_t GOPLUS_ADDR = 0x38;

// Angle the arm parks at after start-up. It is the same angle as BIN2 (see docs/OVERVIEW.md).
static int init_angle = 30;

void ServoSorter::setHardware(IServoDriver *driver, II2CBus *bus)
{
    _driver = driver;
    _bus = bus;
}

String ServoSorter::stateToString(ServoState s)
{
    switch (s)
    {
    case ServoState::Init:
        return "INIT";
    case ServoState::Idle:
        return "IDLE";
    case ServoState::Moving:
        return "MOVING";
    }
    return "UNKNOWN";
}

void ServoSorter::init(StateManager &stateManager)
{
    _driver->begin();
    _driver->writeAngle(SERVO_CHANNEL, init_angle); // neutral

    // Observe the servo state
    stateManager.observeState<ServoState>(
        &servoState,
        "SERVO",
        stateToString);

    stateManager.addStateRule("SERVO", "INIT", "*", ERROR_INFO, "Servo initialized");

    servoState = ServoState::Idle;
}

int ServoSorter::angleFor(Bin::Id bin) const
{
    switch (bin)
    {
    case Bin::BIN2:
        return 30;
    case Bin::BIN3:
        return 60;
    default:
        return 0;
    }
}

// Turns the arm to the angle of the bin. Bin::NONE goes to the error bin instead.
void ServoSorter::moveTo(Bin::Id bin)
{
        if (bin == Bin::NONE)
    {
        // Unknown parcel: send it to the error bin rather than leaving it on the belt
        LOG_WARN("Servo", "no bin for this parcel, sending it to the error bin");
        bin = Bin::ERROR_BIN;
    }

    servoState = ServoState::Moving;

    uint8_t angle = (uint8_t)angleFor(bin);

    LOG_INFO("Servo", "moving to %u degrees", angle);

    _driver->writeAngle(SERVO_CHANNEL, angle);

    servoState = ServoState::Idle;
}

bool ServoSorter::checkConnection()
{
    uint8_t error = _bus->probe(GOPLUS_ADDR);

    if (error == 0)
    {
        LOG_INFO("Servo", "GoPlus2 detected at 0x38");
        return true;
    }
    else
    {
        LOG_ERROR("Servo", "GoPlus2 not detected at 0x38 (I2C error %d)", error);
        return false;
    }
}
