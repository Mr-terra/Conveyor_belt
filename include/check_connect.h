#ifndef CHECK_CONNECT_H
#define CHECK_CONNECT_H

#include <Arduino.h>

class CheckConnect {
public:
    void initialize();
    bool checkConnections();
    bool checkDolibarrConnection();
    bool checkMotorConnection();
    bool checkSorterConnection();
    bool checkRFIDConnection();
};

#endif