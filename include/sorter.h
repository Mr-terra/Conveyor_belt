#ifndef SORTER_H
#define SORTER_H

#include <Arduino.h>

// State constants for sorter
extern const String SORTER_STATE_INITIAL;
extern const String SORTER_STATE_MOVING;
extern const String SORTER_STATE_ERROR;

class Sorter {
public:
    void initialize();
    void moveTo(String direction);
    void stop();
    String getState();
    void setState(String state);
    
    // State conversion function for automatic monitoring
    static String stateToString(String state);
};

#endif