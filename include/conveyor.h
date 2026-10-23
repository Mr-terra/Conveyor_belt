#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>
#include "hal/grbl_motor.h"
#include "hal/i2c_bus.h"

class StateManager;

extern const String CONVEYOR_STATE_STOPPED;
extern const String CONVEYOR_STATE_RUNNING;
extern const String CONVEYOR_STATE_ALARM;

class Conveyor {
public:
    Conveyor(int movement = 1, int speed = 1000);

    // Injects the hardware drivers. Must be called before initialize().
    void setHardware(IGrblMotor *motor, II2CBus *bus);

    void initialize(StateManager& stateManager);
    void start();
    void stop();
    void move(bool forward = true);

    // Health check - verifies I2C communication with the GRBL module
    bool checkConnection();

    String getState() const;
    void setState(String state);  // DEPRECATED - do not use

    void setSpeed(int speed);
    void setMovement(int steps);

    // ✨ KEY FUNCTIONS for automatic monitoring
    String readHardwareState();     // Reads the state from the GRBL module
    void monitorHardware();         // Call in loop()
    void configureStateMonitoring(StateManager& stateManager);

    static String stateToString(String state);
    static void setupStateRules(StateManager& stateManager);

private:
    int _movement;
    int _speed;
    IGrblMotor *_grbl = nullptr;
    II2CBus *_bus = nullptr;
    bool _isRunning;
    String _currentState;
};

#endif
