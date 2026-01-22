#ifndef ERROR_H
#define ERROR_H

#include <Arduino.h>

enum ErrorLevel {
    ERROR_INFO,      // Information/avertissement
    ERROR_WARNING,   // Avertissement non critique
    ERROR_CRITICAL,  // Erreur critique
    ERROR_FATAL      // Erreur fatale
};

class Error {
public:
    Error(String message, ErrorLevel level, String module);
    String getMessage() const;
    ErrorLevel getLevel() const;
    String getModule() const;
    String toString() const;

private:
    String message;
    ErrorLevel level;
    String module;
    unsigned long timestamp;
};

#endif