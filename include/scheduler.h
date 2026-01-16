#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

class Scheduler {
public:
    void initialize();
    void setState(String state);
    String getState();
    void triggerCheckConnect();
    void triggerInitPositions();
    void triggerProduction();
    void triggerStop();
};

#endif