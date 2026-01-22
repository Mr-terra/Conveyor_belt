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

    // Error management - pure observation and reporting
    static void triggerError(String message, ErrorLevel level, String module);
    static bool hasCriticalErrors();
    static void clearCriticalErrors();

    // Callback system for error handling (separation of concerns)
    using ErrorCallback = std::function<void(const Error& error)>;
    using CriticalErrorCallback = std::function<void()>;
    
    static void setErrorCallback(ErrorCallback callback);
    static void setCriticalErrorCallback(CriticalErrorCallback callback);

private:
    String message;
    ErrorLevel level;
    String module;
    unsigned long timestamp;

    // Static members for error tracking
    static bool criticalErrorFlag;
    static ErrorCallback errorCallback;
    static CriticalErrorCallback criticalErrorCallback;

    static void logError(const Error& error);

};

#endif