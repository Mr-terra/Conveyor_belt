#include "log.h"

#include <stdarg.h>
#include <stdio.h>

static const size_t MAX_LINE = 200;

static const char *levelName(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    }
    return "UNKNOWN";
}

void logMessage(LogLevel level, const char *module, const char *format, ...)
{
    char text[MAX_LINE];

    va_list args;
    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);

    Serial.printf("[%s] %s: %s\n", levelName(level), module, text);
}
