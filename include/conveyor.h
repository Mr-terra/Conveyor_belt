#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>
#include <M5Stack.h>
#include <Module_GRBL_13.2.h>

class StateManager;

extern const String CONVEYOR_STATE_STOPPED;
extern const String CONVEYOR_STATE_RUNNING;
extern const String CONVEYOR_STATE_ALARM;

class Conveyor {
public:
    Conveyor(int movement = 1, int speed = 1000, uint8_t i2cAddress = 0x70);
    
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
    Module_GRBL _grbl;
    uint8_t _i2cAddress;
    bool _isRunning;
    String _currentState;
};

#endif