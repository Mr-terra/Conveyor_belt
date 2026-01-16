#ifndef ERROR_MANAGEMENT_H
#define ERROR_MANAGEMENT_H

#include <Arduino.h>

class ErrorManagement {
public:
    void initialize();
    void triggerError(String message);
    bool isErrorDetected();
    bool classifyError();
    void autoRecovery();
    void safetyMeasures();
};

#endif