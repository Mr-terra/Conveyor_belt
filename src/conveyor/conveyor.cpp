// Conveyor: GRBL motor commands, the state read from the hardware every 100 ms,
// and the watchdog for a silent module. The module is at I2C address 0x70.

#include "conveyor.h"
#include "log.h"
#include "state_manager.h"
#include "error.h"

// I2C address of the GRBL 13.2 module.
#define CONVEYOR_ADDR 0x70



// Speed is limited to 1000 here, the largest value the module accepts.
Conveyor::Conveyor(int movement, int speed)
    : _movement(movement), _speed(speed), _isRunning(false)
{
    if (_speed > 1000)
    {
        _speed = 1000;
    }
}

void Conveyor::setHardware(IGrblMotor *motor, II2CBus *bus)
{
    _grbl = motor;
    _bus = bus;
}

String Conveyor::stateToString(ConveyorState state)
{
    switch (state)
    {
    case ConveyorState::Stopped:
        return "STOPPED";
    case ConveyorState::Running:
        return "RUNNING";
    case ConveyorState::Alarm:
        return "ALARM";
    }
    return "UNKNOWN";
}

void Conveyor::setupStateRules(StateManager &stateManager)
{
    // Rules based on real hardware transitions
    stateManager.addStateRule("Conveyor", "STOPPED", "RUNNING", ERROR_INFO, "Motor started (detected from hardware)");
    stateManager.addStateRule("Conveyor", "RUNNING", "STOPPED", ERROR_INFO, "Motor stopped (detected from hardware)");
    stateManager.addStateRule("Conveyor", "RUNNING", "ALARM", ERROR_CRITICAL, "Hardware alarm while running");
    stateManager.addStateRule("Conveyor", "STOPPED", "ALARM", ERROR_CRITICAL, "Hardware alarm while stopped");
    stateManager.addStateRule("Conveyor", "*", "ALARM", ERROR_FATAL, "GRBL ALARM - immediate stop required");
    stateManager.addStateRule("Conveyor", "ALARM", "STOPPED", ERROR_WARNING, "Recovered from alarm - unlocked");
}

void Conveyor::configureStateMonitoring(StateManager &stateManager)
{
    stateManager.observeState(&_currentState, "Conveyor", Conveyor::stateToString);
    setupStateRules(stateManager);
}

// Called once from setup(), after the hardware has been bound.
// Unlocks the module, stops the motor, reads the first state, then registers it with the StateManager.
void Conveyor::initialize(StateManager &stateManager)
{
    // Initialize the GRBL module
    _grbl->init();
    _grbl->setMode("distance");

    // Clear the buffer
    _grbl->readClean();

    // Unlock if needed
    if (_grbl->inLock())
    {
        LOG_WARN("Conveyor", "module locked, unlocking");
        _grbl->unLock();
        delay(100);
    }

    // Stop the motor at startup
    _grbl->setMotor(0, 0, 0, 0);
    _grbl->waitIdle();

    // Read the initial state from the hardware
    readHardwareState();

    LOG_INFO("Conveyor", "initialized, GRBL module 13.2");
    LOG_INFO("Conveyor", "initial state %s", stateToString(_currentState).c_str());

    configureStateMonitoring(stateManager);
}

// Three checks in order: the I2C address answers, the status can be read, and the module is not locked.
bool Conveyor::checkConnection()
{
    // Robust check: test I2C communication with the GRBL module

    // Step 1: basic check of the I2C address
    byte error = _bus->probe(CONVEYOR_ADDR); // Default GRBL address

    if (error != 0)
    {
        LOG_ERROR("Conveyor", "GRBL module not detected at 0x%X (I2C error %d)", CONVEYOR_ADDR, error);
        return false;
    }

    // Step 2: check that the module responds to commands
    String status = _grbl->readStatus();

    if (status.length() == 0)
    {
        LOG_ERROR("Conveyor", "GRBL module does not answer commands");
        return false;
    }

    // Step 3: check that the module is not in an error state
    if (_grbl->inLock())
    {
        LOG_ERROR("Conveyor", "GRBL module is locked, unlock it before use");
                return false;
    }

    LOG_INFO("Conveyor", "GRBL module operational at 0x%X", CONVEYOR_ADDR);
        return true;
}

// Reads the state from the GRBL module and returns it.
// The status letter is mapped to a ConveyorState; a missing answer is counted (see the watchdog below).
ConveyorState Conveyor::readHardwareState()
{
    ConveyorState newState = _currentState;

    // Read the module status (returns a String starting with the state code)
    String status = _grbl->readStatus();

    if (status.length() > 0)
    {
        _missedReplies = 0;
        char stateChar = status[0];

        // Map GRBL codes to conveyor states
        switch (stateChar)
        {
        case 'I': // Idle
            newState = ConveyorState::Stopped;
            break;

        case 'A': // Alarm (locked)
            newState = ConveyorState::Alarm;
            break;

        case 'R': // Run
        case 'J': // Jog
        case 'H': // Hold
        default:  // Any other state = moving or busy
            if (stateChar != 'I' && stateChar != 'A')
            {
                newState = ConveyorState::Running;
            }
            else
            {
                newState = ConveyorState::Stopped;
            }
            break;
        }

    }
    else
    {
        _missedReplies++;

        // Watchdog: after MISSED_REPLIES_LIMIT silent reads the module is treated as failed.
        // The critical error is raised only once, on the read that reaches the limit.
        if (_missedReplies >= MISSED_REPLIES_LIMIT)
        {
            // Silence for too long: do not assume the belt is still running
            newState = ConveyorState::Alarm;
            if (_missedReplies == MISSED_REPLIES_LIMIT)
            {
                Error::triggerError("GRBL module not responding", ERROR_CRITICAL, "Conveyor");
            }
        }
        else if (_grbl->inLock())
        {
            // No response yet, fall back to the helper functions
            newState = ConveyorState::Alarm;
        }
        else if (_grbl->readIdle())
        {
            newState = ConveyorState::Stopped;
        }
        else
        {
            newState = ConveyorState::Running;
        }
    }

    // === STATE UPDATE (automatic) ===
    if (newState != _currentState)
    {
        LOG_INFO("Conveyor", "state %s -> %s", stateToString(_currentState).c_str(), stateToString(newState).c_str());

        _currentState = newState; // ← The only place where the state is changed
        // The StateManager will detect this change automatically
    }

    return newState;
}

// Called on every loop pass. The hardware is read at most every 100 ms.
// The state rules run in the StateManager, not here.
void Conveyor::monitorHardware()
{
    static unsigned long lastCheck = 0;
    unsigned long now = millis();

    if (now - lastCheck >= 100)
    {
        // Read the state from the hardware
        readHardwareState();
        // ↑ That's ALL! The StateManager rules handle the rest

        // Reset the _isRunning flag if the motor is stopped
        if (_currentState == ConveyorState::Stopped)
        {
            _isRunning = false;
        }

        lastCheck = now;
    }
}

// Commands only send orders to the motor. The resulting state comes from the next hardware read.
void Conveyor::start()
{
    // Unlock if needed
    if (_grbl->inLock())
    {
        LOG_INFO("Conveyor", "unlocking module");
        _grbl->unLock();
        delay(100);
    }

    _isRunning = true;
    // The state will switch to RUNNING automatically via readHardwareState()
}

void Conveyor::stop()
{
    _grbl->setMotor(0, 0, 0, 0);
    _grbl->waitIdle(); // Wait until the motor has really stopped
    _isRunning = false;
    // The state will switch to STOPPED automatically via readHardwareState()
}

void Conveyor::move(bool forward)
{
    if (!_isRunning)
    {
        this->start(); // Start the motor if it is not already running
    }

    // The sign of the step gives the direction: positive forward, negative backward.
    int angle = forward ? _movement : -_movement;

    // Called every loop tick: no log here, the state changes are logged by readHardwareState()
    _grbl->setMotor(angle, 0, 0, _speed);
}

void Conveyor::setSpeed(int speed)
{
    _speed = constrain(speed, 0, 1000);
}

void Conveyor::setMovement(int steps)
{
    _movement = steps;
}

ConveyorState Conveyor::getState() const
{
    return _currentState;
}

