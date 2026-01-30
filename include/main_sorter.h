#pragma once
#include <Arduino.h>
#include "state_manager.h"

class SorterController
{
public:
    void init(StateManager &sm);
    void production();

private:
    StateManager *sm = nullptr;
    String sorterState = "INIT_PRODUCTION";

    static String stateToString(String *s);
    static void setupStateRules(StateManager &stateManager);
};
