#include "init_position.h"
#include <Arduino.h>

InitPosition initPosition;

void InitPosition::initialize() {
}

bool InitPosition::initializePositions() {
    // Initialize positions
    bool paramsOk = loadMotorParameters();
    bool positionOk = initializeSorterPosition();

    return paramsOk && positionOk;
}

// Placeholder: no motor parameters are loaded yet. Returns true so the
// scheduler's init step can run; replace with a real GRBL configuration.
bool InitPosition::loadMotorParameters() {
    return true;
}

// Placeholder: the sorter is not homed in hardware yet. Returns true so the
// scheduler's init step can run; replace with a real homing sequence.
bool InitPosition::initializeSorterPosition() {
    return true;
}
