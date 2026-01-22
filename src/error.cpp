#include "error.h"

// Initialize static members
bool Error::criticalErrorFlag = false;
Error::ErrorCallback Error::errorCallback = nullptr;
Error::CriticalErrorCallback Error::criticalErrorCallback = nullptr;

Error::Error(String message, ErrorLevel level, String module)
    : message(message), level(level), module(module), timestamp(millis()) {}

String Error::getMessage() const {
    return message;
}

ErrorLevel Error::getLevel() const {
    return level;
}

String Error::getModule() const {
    return module;
}

String Error::toString() const {
    String levelStr;
    switch (level) {
        case ERROR_INFO: levelStr = "INFO"; break;
        case ERROR_WARNING: levelStr = "WARNING"; break;
        case ERROR_CRITICAL: levelStr = "CRITICAL"; break;
        case ERROR_FATAL: levelStr = "FATAL"; break;
    }
    return "[" + String(timestamp) + "] " + levelStr + " [" + module + "]: " + message;
}

void Error::triggerError(String message, ErrorLevel level, String module) {
    Error error(message, level, module);
    logError(error);
    
    // Pure observation - delegate actions to callbacks
    if (errorCallback) {
        errorCallback(error);
    }
    
    if (level == ERROR_CRITICAL || level == ERROR_FATAL) {
        criticalErrorFlag = true;
        if (criticalErrorCallback) {
            criticalErrorCallback();
        }
    }
}

bool Error::hasCriticalErrors() {
    return criticalErrorFlag;
}

void Error::clearCriticalErrors() {
    criticalErrorFlag = false;
}

void Error::logError(const Error& error) {
    Serial.println(error.toString());
}

void Error::setErrorCallback(ErrorCallback callback) {
    errorCallback = callback;
}

void Error::setCriticalErrorCallback(CriticalErrorCallback callback) {
    criticalErrorCallback = callback;
}