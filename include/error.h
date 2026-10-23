// Error: error reporting, with the latch for critical errors.

#ifndef ERROR_H
#define ERROR_H

#include <Arduino.h>

// Severity of an error.
// CRITICAL and FATAL errors latch: they stay set until clearCriticalErrors() is called.
enum ErrorLevel {
    ERROR_INFO,      // Information/notice
    ERROR_WARNING,   // Non-critical warning
    ERROR_CRITICAL,  // Critical error
    ERROR_FATAL      // Fatal error
};

// One error or event: its level, its module and the time it happened.
// Errors are reported through the static functions, which also run the registered callbacks.
class Error {
public:
    // Creates an error. The timestamp is taken when it is created (millis()).
    Error(String message, ErrorLevel level, String module);
    String getMessage() const;
    ErrorLevel getLevel() const;
    String getModule() const;
    String toString() const;

    // Error management - pure observation and reporting
    // Logs the error, calls the error callback, and latches the critical flag for CRITICAL and FATAL.
    static void triggerError(String message, ErrorLevel level, String module);
    // True while a CRITICAL or FATAL error has not been cleared.
    static bool hasCriticalErrors();
    // Clears the latch. In normal operation only the operator's Start does this.
    static void clearCriticalErrors();

    // Callback system for error handling (separation of concerns)
    // Called for every error. It observes the error; it does not change it.
    using ErrorCallback = std::function<void(const Error& error)>;
    using CriticalErrorCallback = std::function<void()>;
    
    // Registers the callback for every error. Pass nullptr to remove it.
    static void setErrorCallback(ErrorCallback callback);
    // Registers a callback that runs once for each CRITICAL or FATAL error.
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

    // Prints the error to the serial console.
    static void logError(const Error& error);

};

#endif