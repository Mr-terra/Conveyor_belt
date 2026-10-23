// InitPosition: placeholder for the position initialization before production.

#include "init_position.h"
#include <Arduino.h>


// Nothing to set up yet. The method is kept so that the start-up sequence does not change
// when the real steps are added.
void InitPosition::initialize() {
}

// Both steps must pass for the scheduler to move on to PRODUCTION.
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
