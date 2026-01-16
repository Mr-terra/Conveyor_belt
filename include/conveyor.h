#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>

class Conveyor {
public:
    void initialize();
    void start();
    void stop();
    String getState();
    void setState(String state);
};

#endif