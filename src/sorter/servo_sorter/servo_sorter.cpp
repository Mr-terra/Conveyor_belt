#include "servo_sorter.h"

// Servo channel on the GoPlus2 board and I2C address of the board
static const uint8_t SERVO_CHANNEL = 0;
static const uint8_t GOPLUS_ADDR = 0x38;

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

void ServoSorter::moveTo(Bin::Id bin)
{
    Serial.println((int)bin);

    if (bin == Bin::NONE)
    {
        // Unknown parcel: send it to the error bin rather than leaving it on the belt
        Serial.println("No bin for this parcel - sending it to the error bin");
        bin = Bin::ERROR_BIN;
    }

    servoState = ServoState::Moving;

    uint8_t angle = (uint8_t)angleFor(bin);

    Serial.println(angle);

    _driver->writeAngle(SERVO_CHANNEL, angle);

    servoState = ServoState::Idle;
}

bool ServoSorter::checkConnection()
{
    uint8_t error = _bus->probe(GOPLUS_ADDR);

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
