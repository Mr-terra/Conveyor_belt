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

class Conveyor {
public:
    // Reads, at 100 ms each, without an answer before the module is treated as failed
    static constexpr int MISSED_REPLIES_LIMIT = 10;

    Conveyor(int movement = 1, int speed = 1000);

    // Injects the hardware drivers. Must be called before initialize().
    void setHardware(IGrblMotor *motor, II2CBus *bus);

    void initialize(StateManager& stateManager);
    void start();
    void stop();
    void move(bool forward = true);

    // Health check - verifies I2C communication with the GRBL module
    bool checkConnection();

    ConveyorState getState() const;

    void setSpeed(int speed);
    void setMovement(int steps);

    // ✨ KEY FUNCTIONS for automatic monitoring
    ConveyorState readHardwareState();     // Reads the state from the GRBL module
    void monitorHardware();         // Call in loop()
    void configureStateMonitoring(StateManager& stateManager);

    static String stateToString(ConveyorState state);
    static void setupStateRules(StateManager& stateManager);

private:
    int _movement;
    int _speed;
    IGrblMotor *_grbl = nullptr;
    II2CBus *_bus = nullptr;
    bool _isRunning;
    ConveyorState _currentState = ConveyorState::Stopped;

    // Consecutive status reads without an answer from the GRBL module
    int _missedReplies = 0;
};

#endif
