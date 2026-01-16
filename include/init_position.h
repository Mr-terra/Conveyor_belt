#ifndef INIT_POSITION_H
#define INIT_POSITION_H

#include <Arduino.h>

class InitPosition {
public:
    void initialize();
    bool initializePositions();
    bool loadMotorParameters();
    bool initializeSorterPosition();
};

#endif