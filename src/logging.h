#ifndef clox_logging_h
#define clox_logging_h

#include <stdio.h>
#include <stdarg.h>
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
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} LogLevel;

static const char *LOG_LEVEL_NAMES[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
};

void set_log_level(LogLevel level);
LogLevel current_log_level();
void set_log_stream(FILE *file);
void logger(LogLevel level, const char *file, int line, const char *fmt, ...);

#define log_trace(...) logger(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) logger(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) logger(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) logger(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) logger(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

static inline const char* log_level_name(int level) {
    return LOG_LEVEL_NAMES[level];
}

#endif