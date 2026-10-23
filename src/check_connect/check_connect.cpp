// CheckConnect: runs the connection checks before production (see check_connect.h).

#include "check_connect.h"
#include "log.h"
#include "conveyor.h"
#include "main_sorter.h"
#include "dolibarr_api.h"
#include <Arduino.h>


void CheckConnect::setDolibarrAPI(DolibarrAPI *api)
{
    _dolibarrApi = api;
}

void CheckConnect::setConveyor(Conveyor *conv)
{
    _conveyor = conv;
}

void CheckConnect::setSorterController(SorterController *sorter)
{
    _sorterController = sorter;
}

// Checks that all dependencies are configured
bool CheckConnect::areDependenciesConfigured()
{
    bool allConfigured = (_conveyor != nullptr) &&
                         (_sorterController != nullptr) &&
                         (_dolibarrApi != nullptr);

    if (!allConfigured)
    {
        LOG_WARN("CheckConnect", "dependencies not configured:");
        if (_conveyor == nullptr)
            LOG_WARN("CheckConnect", "conveyor not set");
        if (_sorterController == nullptr)
            LOG_WARN("CheckConnect", "sorter controller not set");
        if (_dolibarrApi == nullptr)
            LOG_WARN("CheckConnect", "Dolibarr API not set");
    }

    return allConfigured;
}

// Runs the three checks. Every check runs even when an earlier one failed, so the log shows all failures.
bool CheckConnect::checkConnections()
{

    // Check that all dependencies are configured
    if (!areDependenciesConfigured())
    {
        LOG_ERROR("CheckConnect", "check aborted: dependencies not configured");
        return false;
    }

    // Check all connections using instance methods
    bool dolibarrOk = checkDolibarrConnection();
    bool motorOk = checkMotorConnection();
    bool sorterOk = checkSorterConnection();

    bool allOk = dolibarrOk && motorOk && sorterOk;
    if (allOk)
    {
        LOG_INFO("CheckConnect", "all connections OK");
    }
    else
    {
        LOG_ERROR("CheckConnect", "connection check failed");
    }

    return allOk;
}

// Dolibarr is optional at this level: without an API object the check is skipped and passes.
bool CheckConnect::checkDolibarrConnection()
{

    if (_dolibarrApi == nullptr)
    {
        LOG_WARN("CheckConnect", "Dolibarr API not configured, check skipped");
        return true; // Assume OK if not configured
    }

    bool connected = _dolibarrApi->checkConnection();

    if (connected)
    {
        LOG_INFO("CheckConnect", "Dolibarr connection OK");
        return true;
    }
    else
    {
        LOG_ERROR("CheckConnect", "Dolibarr connection failed");
        return false;
    }
}

// The conveyor performs its own I2C and lock checks (see Conveyor::checkConnection()).
bool CheckConnect::checkMotorConnection()
{

    if (_conveyor == nullptr)
    {
        LOG_ERROR("CheckConnect", "conveyor not initialized");
        return false;
    }

    return _conveyor->checkConnection();
}

// The sorter performs its own checks for the RFID reader and the servo board.
bool CheckConnect::checkSorterConnection()
{

    if (_sorterController == nullptr)
    {
        LOG_ERROR("CheckConnect", "sorter controller not initialized");
        return false;
    }

    return _sorterController->checkConnection();
}
