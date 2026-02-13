#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

// Forward declarations pour éviter les includes circulaires
class CheckConnect;
class InitPosition;
class SorterController;
class Conveyor;

// State constants for scheduler
extern const String STATE_WAITING;
extern const String STATE_CHECK_CONNECT;
extern const String STATE_INIT_POSITIONS;
extern const String STATE_PRODUCTION;
extern const String STATE_ERROR;

class Scheduler {
public:
    void initialize();
    void setState(String state);
    String getState();
    void triggerCheckConnect();
    void triggerInitPositions();
    void triggerProduction();
    void triggerStop();
    
    // Configuration des dépendances (DOIT être appelé après initialize)
    void setDependencies(
        CheckConnect* checkConnect,
        InitPosition* initPosition,
        SorterController* sorterController,
        Conveyor* conveyor
    );
    
    // Méthode principale pour exécuter la logique de l'état courant
    void executeCurrentState();
    
    // State conversion function for automatic monitoring
    static String stateToString(String state);

private:
    String _current_state;
};

#endif