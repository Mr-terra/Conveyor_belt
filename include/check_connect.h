#ifndef CHECK_CONNECT_H
#define CHECK_CONNECT_H

#include <Arduino.h>

// Forward declarations to avoid circular includes
class DolibarrAPI;
class Conveyor;
class SorterController;

class CheckConnect {
public:
    bool checkConnections();
    
    // Individual methods (can be used separately)
    bool checkDolibarrConnection();
    bool checkMotorConnection();
    bool checkSorterConnection();
    
    // Setters for the dependencies (MUST be called before checkConnections)
    void setDolibarrAPI(DolibarrAPI* api);
    void setConveyor(Conveyor* conveyor);
    void setSorterController(SorterController* sorter);
    
    // Checks that all dependencies are configured
    bool areDependenciesConfigured();

private:
    DolibarrAPI* dolibarrApi = nullptr;
    Conveyor* conveyor = nullptr;
    SorterController* sorterController = nullptr;
};

#endif