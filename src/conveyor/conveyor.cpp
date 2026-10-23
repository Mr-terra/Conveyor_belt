#include "conveyor.h"
#include "state_manager.h"
#include "error.h"

#define CONVEYOR_ADDR 0x70

Conveyor conveyor(10, 250);


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
        Serial.println("⚠️ Module locked, unlocking...");
        _grbl->unLock();
        delay(100);
    }

    // Stop the motor at startup
    _grbl->setMotor(0, 0, 0, 0);
    _grbl->waitIdle();

    // Read the initial state from the hardware
    readHardwareState();

    Serial.println("✅ Conveyor initialized with Module GRBL 13.2");
    Serial.println(stateToString(_currentState));

    configureStateMonitoring(stateManager);
}

bool Conveyor::checkConnection()
{
    // Robust check: test I2C communication with the GRBL module

    // Step 1: basic check of the I2C address
    byte error = _bus->probe(CONVEYOR_ADDR); // Default GRBL address

    if (error != 0)
    {
        Serial.printf("❌ GRBL Motor not detected at 0x%X (I2C error: %d)\n", CONVEYOR_ADDR, error);
        return false;
    }

    // Step 2: check that the module responds to commands
    String status = _grbl->readStatus();

    if (status.length() == 0)
    {
        Serial.println("❌ GRBL Motor detected but not responding to commands");
        return false;
    }

    // Step 3: check that the module is not in an error state
    if (_grbl->inLock())
    {
        Serial.println("❌ GRBL Motor is in LOCK/ALARM state");
        Serial.println("   Module needs to be unlocked before use");
        return false;
    }

    Serial.println("✅ GRBL Motor detected and operational at " + String(CONVEYOR_ADDR, HEX));
    Serial.println(status);
    return true;
}

// ✨ CORE OF THE SYSTEM: reading the state from the hardware
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

        // Debug: print the raw code
    }
    else
    {
        _missedReplies++;

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
        Serial.print("🔄 Hardware state change: ");
        Serial.print(stateToString(_currentState));
        Serial.print(" → ");
        Serial.println(stateToString(newState));

        _currentState = newState; // ← The only place where the state is changed
        // The StateManager will detect this change automatically
    }

    return newState;
}

// ✨ SIMPLIFIED MONITORING - NO manual triggerError
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

// ✨ SIMPLIFIED COMMANDS (no manual setState)
void Conveyor::start()
{
    // Unlock if needed
    if (_grbl->inLock())
    {
        Serial.println("🔓 Unlocking module...");
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

    // Increase the _movement value for a visible movement
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

