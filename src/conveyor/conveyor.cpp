#include "conveyor.h"
#include "state_manager.h"
#include "error.h"

#define CONVEYOR_ADDR 0x70

Conveyor conveyor(10, 250);

// States based on the real states of the GRBL module
const String CONVEYOR_STATE_STOPPED = "STOPPED"; // 'I' (Idle)
const String CONVEYOR_STATE_RUNNING = "RUNNING"; // Anything other than 'I' or 'A'
const String CONVEYOR_STATE_ALARM = "ALARM";     // 'A' (Alarm/Lock)

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

String Conveyor::stateToString(String state)
{
    if (state == CONVEYOR_STATE_STOPPED)
        return "STOPPED";
    if (state == CONVEYOR_STATE_RUNNING)
        return "RUNNING";
    if (state == CONVEYOR_STATE_ALARM)
        return "ALARM";
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
    Serial.print("📊 Initial state: ");
    Serial.println(_currentState);

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
        Serial.println("   Possible causes: ");
        Serial.println("   - Module not powered");
        Serial.println("   - Wrong I2C address");
        Serial.println("   - I2C bus not initialized");
        Serial.println("   - Wiring issue");
        return false;
    }

    // Step 2: check that the module responds to commands
    String status = _grbl->readStatus();

    if (status.length() == 0)
    {
        Serial.println("❌ GRBL Motor detected but not responding to commands");
        Serial.println("   Possible causes: ");
        Serial.println("   - Module not properly initialized");
        Serial.println("   - Communication error");
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
    Serial.print("📊 Current status: ");
    Serial.println(status);
    return true;
}

// ✨ CORE OF THE SYSTEM: reading the state from the hardware
String Conveyor::readHardwareState()
{
    String newState;

    // Read the module status (returns a String starting with the state code)
    String status = _grbl->readStatus();

    if (status.length() > 0)
    {
        char stateChar = status[0];

        // Map GRBL codes to conveyor states
        switch (stateChar)
        {
        case 'I': // Idle
            newState = CONVEYOR_STATE_STOPPED;
            break;

        case 'A': // Alarm (locked)
            newState = CONVEYOR_STATE_ALARM;
            break;

        case 'R': // Run
        case 'J': // Jog
        case 'H': // Hold
        default:  // Any other state = moving or busy
            if (stateChar != 'I' && stateChar != 'A')
            {
                newState = CONVEYOR_STATE_RUNNING;
            }
            else
            {
                newState = CONVEYOR_STATE_STOPPED;
            }
            break;
        }

        // Debug: print the raw code
        if (newState != _currentState)
        {
            Serial.print("📡 Raw status: '");
            Serial.print(stateChar);
            Serial.print("' (");
            Serial.print((int)stateChar);
            Serial.println(")");
        }
    }
    else
    {
        // No response, fall back to the helper functions
        if (_grbl->inLock())
        {
            newState = CONVEYOR_STATE_ALARM;
        }
        else if (_grbl->readIdle())
        {
            newState = CONVEYOR_STATE_STOPPED;
        }
        else
        {
            newState = CONVEYOR_STATE_RUNNING;
        }
    }

    // === STATE UPDATE (automatic) ===
    if (newState != _currentState)
    {
        Serial.print("🔄 Hardware state change: ");
        Serial.print(_currentState);
        Serial.print(" → ");
        Serial.println(newState);

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
        if (_currentState == CONVEYOR_STATE_STOPPED)
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
    Serial.println("✅ Conveyor start command sent");
    // The state will switch to RUNNING automatically via readHardwareState()
}

void Conveyor::stop()
{
    _grbl->setMotor(0, 0, 0, 0);
    _grbl->waitIdle(); // Wait until the motor has really stopped
    _isRunning = false;
    Serial.println("🛑 Conveyor stop command sent");
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

String Conveyor::getState() const
{
    return _currentState;
}

