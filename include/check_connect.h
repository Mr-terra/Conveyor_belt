// CheckConnect: the connection checks that must pass before production can start.

#ifndef CHECK_CONNECT_H
#define CHECK_CONNECT_H

#include <Arduino.h>

// Forward declarations to avoid circular includes
class DolibarrAPI;
class Conveyor;
class SorterController;

// Runs the checks that must pass before production: the Dolibarr API, the GRBL motor,
// and the sorter (RFID reader and servo board).
class CheckConnect {
public:
    // Runs every check. Returns true only if all of them pass.
    bool checkConnections();
    
    // Individual methods (can be used separately)
    // Passes when the Dolibarr API answers HTTP 200.
    // If no API is configured the check is skipped and counts as passed (the skip is logged).
    bool checkDolibarrConnection();
    // Passes when the GRBL motor answers on I2C and is not locked.
    bool checkMotorConnection();
    // Passes when the RFID reader and the servo board both answer.
    bool checkSorterConnection();
    
    // Setters for the dependencies (MUST be called before checkConnections)
    void setDolibarrAPI(DolibarrAPI* api);
    void setConveyor(Conveyor* conveyor);
    void setSorterController(SorterController* sorter);
    
    // Checks that all dependencies are configured
    bool areDependenciesConfigured();

private:
    DolibarrAPI* _dolibarrApi = nullptr;
    Conveyor* _conveyor = nullptr;
    SorterController* _sorterController = nullptr;
};

#endif