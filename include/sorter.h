#ifndef SORTER_H
#define SORTER_H

#include <Arduino.h>

class Sorter {
public:
    void initialize();
    void moveTo(String direction);
    void stop();
    String getState();
    void setState(String state);
};

#endif