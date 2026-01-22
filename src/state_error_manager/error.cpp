#include "error.h"

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