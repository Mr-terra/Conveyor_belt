#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

// State constants for scheduler
extern const String STATE_WAITING;
extern const String STATE_CHECK_CONNECT;
extern const String STATE_INIT_POSITIONS;
extern const String STATE_PRODUCTION;

class Scheduler {
public:
    void initialize();
    void setState(String state);
    String getState();
    void triggerCheckConnect();
    void triggerInitPositions();
    void triggerProduction();
    void triggerStop();
    
    // State conversion function for automatic monitoring
    static String stateToString(String state);
};

#endif