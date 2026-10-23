// InitPosition: the position initialization step, run before production.

#ifndef INIT_POSITION_H
#define INIT_POSITION_H

#include <Arduino.h>

// Initialization before production. Both steps are placeholders that return true, so the
// state machine can run end to end. Replace their bodies with the real hardware steps.
class InitPosition {
public:
    void initialize();
    bool initializePositions();
    bool loadMotorParameters();
    bool initializeSorterPosition();
};

#endif