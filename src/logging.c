#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "allocator.h"
#include "assert.h"
#include "logging.h"

#ifndef DEFAULT_LOG_LEVEL
// TODO: set to LOG_LEVEL_WARN at some point
#define DEFAULT_LOG_LEVEL LOG_LEVEL_INFO
#endif

#ifndef DEFAULT_LOG_STREAM
#define DEFAULT_LOG_STREAM stderr
#endif

#pragma region Declare

static void init_log_event(LogEvent *event, void *stream);
static void write_log_event(LogEvent *event);
static inline bool is_valid_log_level(int level);

#pragma endregion

#pragma region Public

void logger_init(Logger *logger, FILE *stream, LogLevel level) {
    Allocator *alloc = (Allocator *)malloc(sizeof(Allocator));
    Assert(alloc != NULL);
    allocator_init(alloc, logger);

    *logger = (Logger){ .stream = stream != NULL ? stream : DEFAULT_LOG_STREAM,
                        .level = is_valid_log_level(level) ? level : DEFAULT_LOG_LEVEL,
                        .alloc = alloc };
}

void logger_destroy(Logger *logger) {
    allocator_destroy(logger->alloc);
    logger->alloc = NULL;
    // TODO: close file?
}

void logger_emit(Logger *logger, LogLevel level, const char *file, int line, const char *fmt, ...) {
    if (level >= logger->level) {
        LogEvent event = { .fmt = fmt, .file = file, .line = line, .level = level };
        init_log_event(&event, logger->stream != NULL ? logger->stream : DEFAULT_LOG_STREAM);
        va_start(event.ap, fmt);
        write_log_event(&event);
        va_end(event.ap);
    }
}

#pragma endregion

#pragma region Private

static void init_log_event(LogEvent *event, void *stream) {
    struct timeval tv = { 0 };
    Assert(gettimeofday(&tv, NULL) == 0);
    Assert(localtime_r(&tv.tv_sec, &event->time) != NULL);
    event->time_ms = tv.tv_usec / 1000;
    event->stream = stream;
}

static void write_log_event(LogEvent *event) {
    char buf[64];
    size_t len = strftime(buf, sizeof(buf) - 1, "%H:%M:%S", &event->time);
    len += snprintf(buf + len, sizeof(buf) - len, ".%03d", event->time_ms);
    buf[len] = '\0';
    fprintf(event->stream, "%s %-5s %s:%d: ", buf, LOG_LEVEL_NAMES[event->level], event->file,
            event->line);
    vfprintf(event->stream, event->fmt, event->ap);
    fprintf(event->stream, "\n");
    fflush(event->stream);
}

static inline bool is_valid_log_level(int level) {
    return level >= LOG_LEVEL_MIN && level <= LOG_LEVEL_MAX;
}

#pragma endregion
