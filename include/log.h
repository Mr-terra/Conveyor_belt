#pragma once

#include <Arduino.h>

// Severity of a log line. Each line is printed as "[LEVEL] module: message".
enum class LogLevel : uint8_t
{
    Info,
    Warning,
    Error
};

// Prints one printf-style line to the serial console.
#if defined(__GNUC__)
void logMessage(LogLevel level, const char *module, const char *format, ...)
    __attribute__((format(printf, 3, 4)));
#else
void logMessage(LogLevel level, const char *module, const char *format, ...);
#endif

#define LOG_INFO(module, ...) logMessage(LogLevel::Info, module, __VA_ARGS__)
#define LOG_WARN(module, ...) logMessage(LogLevel::Warning, module, __VA_ARGS__)
#define LOG_ERROR(module, ...) logMessage(LogLevel::Error, module, __VA_ARGS__)
