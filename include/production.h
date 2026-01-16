#ifndef PRODUCTION_H
#define PRODUCTION_H

#include <Arduino.h>

class Production {
public:
    void initialize();
    void startProduction();
    void stopProduction();
    String readPackage();
};

#endif