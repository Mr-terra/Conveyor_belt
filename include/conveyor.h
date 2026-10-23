// Conveyor: drives the belt through the GRBL motor module and tracks its state.

#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>
#include "hal/grbl_motor.h"
#include "hal/i2c_bus.h"

class StateManager;

// State of the GRBL motor module. Conveyor::stateToString() gives the name used in the logs.
enum class ConveyorState : uint8_t
{
    Stopped,
    Running,
    Alarm
};

// Drives the belt and tracks its state.
// The state is read from the GRBL module on every check; it is never assumed after a command
// (see readHardwareState()).
class Conveyor {
public:
    // Reads, at 100 ms each, without an answer before the module is treated as failed
    static constexpr int MISSED_REPLIES_LIMIT = 10;

    // movement: number of steps per move() call.
    // speed: motor speed, 0 to 1000 (values above 1000 are clamped in the constructor).
    Conveyor(int movement = 1, int speed = 1000);

    // Injects the hardware drivers. Must be called before initialize().
    void setHardware(IGrblMotor *motor, II2CBus *bus);

    // Initializes the module, unlocks it if needed, reads the first state,
    // and registers the state with the StateManager.
    void initialize(StateManager& stateManager);
    // Unlocks the module if needed and marks the belt as running.
    // The real state comes from the next hardware read.
    void start();
    // Stops the motor and waits until the module reports that it is idle.
    void stop();
    // Moves the belt one step. Starts the motor first if it is not running yet.
    void move(bool forward = true);

    // Health check - verifies I2C communication with the GRBL module
    bool checkConnection();

    // Returns the last state read from the hardware.
    ConveyorState getState() const;

    // Sets the motor speed, clamped to 0 to 1000.
    void setSpeed(int speed);
    // Sets the number of steps per move() call.
    void setMovement(int steps);

    // ✨ KEY FUNCTIONS for automatic monitoring
    ConveyorState readHardwareState();     // Reads the state from the GRBL module
    void monitorHardware();         // Call in loop()
    void configureStateMonitoring(StateManager& stateManager);

    // Name of a state, as used in the logs and by the state rules.
    static String stateToString(ConveyorState state);
    // Registers the rules for the conveyor's state changes (info, critical or fatal).
    // See StateManager for how a rule is applied.
    static void setupStateRules(StateManager& stateManager);

private:
    int _movement;
    int _speed;
    IGrblMotor *_grbl = nullptr;
    II2CBus *_bus = nullptr;
    // True between start() and stop(), so that move() does not restart a running motor.
    bool _isRunning;
    ConveyorState _currentState = ConveyorState::Stopped;

    // Consecutive status reads without an answer from the GRBL module
    int _missedReplies = 0;
};

#endif
