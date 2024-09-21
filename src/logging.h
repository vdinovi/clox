#ifndef clox_logging_h
#define clox_logging_h

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

typedef struct LogEvent {
    va_list ap;
    const char *fmt;
    const char *file;
    struct tm time;
    int time_ms;
    void *stream;
    int line;
    int level;
} LogEvent;

typedef void (*LogFn)(LogEvent *ev);

typedef enum LogLevel {
    _LOG_LEVEL_MINIMUM, // do not use
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    _LOG_LEVEL_MAXIMUM, // do not use
} LogLevel;

#define LOG_LEVEL_MIN LOG_LEVEL_TRACE
#define LOG_LEVEL_MAX LOG_LEVEL_ERROR

static const char *LOG_LEVEL_NAMES[] = {
    "_MINIMUM", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "_MAXIMUM",
};
typedef struct Logger {
    FILE *stream;
    LogLevel level;
} Logger;

void logger_init(Logger *logger, FILE *stream, LogLevel level);
void logger_destroy(Logger *logger);
void logger_emit(Logger *logger, LogLevel level, const char *file, int line, const char *fmt, ...);

#define TRACE(logger, ...) logger_emit((logger), LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define DEBUG(logger, ...) logger_emit((logger), LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(logger, ...)  logger_emit((logger), LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(logger, ...)  logger_emit((logger), LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(logger, ...) logger_emit((logger), LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif