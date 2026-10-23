#pragma once
#include <Arduino.h>
#include "sorter_types.h"
#include "state_manager.h"

class ServoSorter
{
public:
    void init(StateManager &stateManager);
    void moveTo(Bin::Id bin);
    
    // Health check - verifies that the servo responds
    static bool checkConnection();

private:
    int angleFor(Bin::Id bin) const;

    // Observable internal state
    String servoState = "INIT";

    // Helper for StateManager
    static String stateToString(String s);
};
