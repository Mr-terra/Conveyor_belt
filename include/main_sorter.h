#pragma once
#include <Arduino.h>
#include "state_manager.h"

class SorterController
{
public:
    void initialize(StateManager &sm);
    void production();

    // Health check - verifies the whole sorting system
    bool checkConnection();

    void setState(const char *newState);

private:
    StateManager *sm = nullptr;
    String sorterState = "INIT_PRODUCTION";

    static String stateToString(String *s);
    static void setupStateRules(StateManager &stateManager);
};

extern SorterController sorterController;