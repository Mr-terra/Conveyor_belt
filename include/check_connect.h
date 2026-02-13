#ifndef CHECK_CONNECT_H
#define CHECK_CONNECT_H

#include <Arduino.h>

// Forward declarations pour éviter les includes circulaires
class DolibarrAPI;
class Conveyor;
class SorterController;

class CheckConnect {
public:
    bool checkConnections();
    
    // Méthodes individuelles (peuvent être utilisées séparément)
    bool checkDolibarrConnection();
    bool checkMotorConnection();
    bool checkSorterConnection();
    
    // Setters pour les dépendances (DOIVENT être appelés avant checkConnections)
    void setDolibarrAPI(DolibarrAPI* api);
    void setConveyor(Conveyor* conveyor);
    void setSorterController(SorterController* sorter);
    
    // Vérification que toutes les dépendances sont configurées
    bool areDependenciesConfigured();

private:
    DolibarrAPI* dolibarrApi = nullptr;
    Conveyor* conveyor = nullptr;
    SorterController* sorterController = nullptr;
};

#endif