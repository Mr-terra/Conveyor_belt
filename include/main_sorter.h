// SorterController: one sorting cycle, from reading a card to moving the diverter.

#pragma once
#include <Arduino.h>
#include "state_manager.h"
#include "addresser.h"
#include "reader.h"
#include "servo_sorter.h"
#include "hal/i2c_bus.h"
#include "hal/rfid_driver.h"
#include "hal/servo_driver.h"

// States of one sorting cycle. SorterController::stateToString() gives the name used by the rules and logs.
enum class SorterState : uint8_t
{
    InitProduction,
    Waiting,
    RfidRead,
    RfidReadingError
};

// Owns the RFID reader and the servo sorter, and runs one sorting cycle per call of production().
class DolibarrAPI;

class SorterController
{
public:
    // Injects the hardware drivers. Must be called before initialize().
    void setHardware(IRfidDriver *rfidDriver, IServoDriver *servoDriver, II2CBus *bus);

    // Gives the addresser the Dolibarr client it needs to look products up.
    void setDolibarrAPI(DolibarrAPI *api);

    // Starts both devices, registers the sorter state with the StateManager, and sets WAITING.
    void initialize(StateManager &sm);
    // One sorting cycle: read a card, find its bin, move the servo.
    // Does nothing when no card is present.
    void production();

    // Health check - verifies the whole sorting system
    bool checkConnection();

    // Sets the step of the cycle. The StateManager reports the change.
    void setState(SorterState newState);

private:
    // The devices and the addresser are owned by the sorter.
    RFIDReader _rfid;
    ServoSorter _servo;
    Addresser _addresser;

    StateManager *sm = nullptr;
    // Current step of the sorting cycle.
    SorterState sorterState = SorterState::InitProduction;

    // Name of a step, as used by the state rules and the logs.
    static String stateToString(SorterState s);
    // Rules that report each step of a cycle (waiting, card read, read error).
    static void setupStateRules(StateManager &stateManager);
};
