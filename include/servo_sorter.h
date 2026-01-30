#pragma once
#include <Arduino.h>
#include "sorter_types.h"
#include "state_manager.h"

class ServoSorter
{
public:
    void init(StateManager &stateManager);
    void moveTo(Bin::Id bin);

private:
    int angleFor(Bin::Id bin) const;

    // État interne observable
    String servoState = "INIT";

    // utilitaire pour StateManager
    static String stateToString(String s);
};
