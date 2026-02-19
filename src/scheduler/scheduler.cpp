#include "scheduler.h"
#include "main_sorter.h"
#include "check_connect.h"
#include "init_position.h"
#include "conveyor.h"
#include <Arduino.h>

Scheduler scheduler;

// State constants for scheduler (now declared in header)
const String STATE_WAITING = "WAITING";
const String STATE_CHECK_CONNECT = "CHECK_CONNECT";
const String STATE_INIT_POSITIONS = "INIT_POSITIONS";
const String STATE_PRODUCTION = "PRODUCTION";
const String STATE_ERROR = "ERROR";

// State conversion function for automatic monitoring
String Scheduler::stateToString(String state)
{
    if (state == STATE_WAITING)
        return "WAITING";
    if (state == STATE_CHECK_CONNECT)
        return "CHECK_CONNECT";
    if (state == STATE_INIT_POSITIONS)
        return "INIT_POSITIONS";
    if (state == STATE_PRODUCTION)
        return "PRODUCTION";
    if (state == STATE_ERROR)
        return "ERROR";
    return "UNKNOWN";
}

// Pointeurs vers les dépendances
static CheckConnect *_checkConnect = nullptr;
static InitPosition *_initPosition = nullptr;
static SorterController *_sorterController = nullptr;
static Conveyor *_conveyor = nullptr;

void Scheduler::initialize()
{
    _current_state = STATE_WAITING;
    Serial.println("Scheduler initialized. State: WAITING");
}

void Scheduler::setDependencies(
    CheckConnect *checkConnect,
    InitPosition *initPosition,
    SorterController *sorterController,
    Conveyor *conveyor)
{
    _checkConnect = checkConnect;
    _initPosition = initPosition;
    _sorterController = sorterController;
    _conveyor = conveyor;
}

void Scheduler::setState(String state)
{
    _current_state = state;
    Serial.print("Scheduler state changed to: ");
    Serial.println(state);
}

String Scheduler::getState()
{
    return _current_state;
}

void Scheduler::triggerCheckConnect()
{
    if (_checkConnect == nullptr)
    {
        Serial.println("❌ Cannot trigger CHECK_CONNECT: CheckConnect not configured");
        setState(STATE_ERROR);
        return;
    }
    setState(STATE_CHECK_CONNECT);
}

void Scheduler::triggerInitPositions()
{
    if (_initPosition == nullptr)
    {
        Serial.println("❌ Cannot trigger INIT_POSITIONS: InitPosition not configured");
        setState(STATE_ERROR);
        return;
    }
    setState(STATE_INIT_POSITIONS);
}

void Scheduler::triggerProduction()
{
    if (_sorterController == nullptr || _conveyor == nullptr)
    {
        Serial.println("❌ Cannot trigger PRODUCTION: Dependencies not configured");
        setState(STATE_ERROR);
        return;
    }
    setState(STATE_PRODUCTION);
}

void Scheduler::triggerStop()
{
    setState(STATE_WAITING);
    if (_conveyor != nullptr)
    {
        _conveyor->stop();
    }
}

// Méthode principale pour exécuter la logique de l'état courant
void Scheduler::executeCurrentState()
{
    if (_current_state == STATE_CHECK_CONNECT)
    {
        if (_checkConnect != nullptr)
        {
            bool connectionsOk = _checkConnect->checkConnections();
            if (connectionsOk)
            {
                triggerInitPositions();
            }
            else
            {
                Serial.println("❌ Connection check failed - returning to WAITING");
                triggerStop();
            }
        }
    }
    else if (_current_state == STATE_INIT_POSITIONS)
    {
        if (_initPosition != nullptr)
        {
            bool initOk = _initPosition->initializePositions();
            if (initOk)
            {
                triggerProduction();
            }
            else
            {
                Serial.println("❌ Position initialization failed - returning to WAITING");
                triggerStop();
            }
        }
    }
    else if (_current_state == STATE_PRODUCTION)
    {
        if (_conveyor != nullptr)
        {
            _conveyor->start();
            _conveyor->move(true); // Mouvement continu
        }
        if (_sorterController != nullptr)
        {
            _sorterController->production();
        }
    }
    // STATE_WAITING et STATE_ERROR ne nécessitent pas d'action spécifique
}