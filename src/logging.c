#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>

#include "logging.h"
#include "assert.h"

#ifndef DEFAULT_LOG_LEVEL
// TODO: set to LOG_WARN at some point
#define DEFAULT_LOG_LEVEL LOG_INFO
#endif

#ifndef DEFAULT_LOG_STREAM
#define DEFAULT_LOG_STREAM stderr
#endif

#pragma region Declare

static struct {
    FILE *stream;
    LogLevel level;
} state = {
    .stream = NULL,
    .level = DEFAULT_LOG_LEVEL,
};

static void init_log_event(LogEvent *event, void *stream);
static void write_log_event(LogEvent *event);

#pragma endregion

#pragma region Public

void set_log_level(LogLevel level) {
    state.level = level;
}

LogLevel current_log_level() {
    return state.level;
}

void set_log_stream(FILE *file) {
    state.stream = file;
}

void logger(LogLevel level, const char *file, int line, const char *fmt, ...) {
    LogEvent event = {
        .fmt = fmt,
        .file = file,
        .line = line,
        .level = level,
    };

    if (level >= state.level) {
        init_log_event(&event, state.stream != NULL ? state.stream : DEFAULT_LOG_STREAM);
        va_start(event.ap, fmt);
        write_log_event(&event);
        va_end(event.ap);
    }
}

#pragma endregion

#pragma region Private

static void init_log_event(LogEvent *event, void *stream) {
    struct timeval tv = {0};
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
    fprintf(event->stream, "%s %-5s %s:%d: ", buf, log_level_name(event->level), event->file, event->line);
    vfprintf(event->stream, event->fmt, event->ap);
    fprintf(event->stream, "\n");
    fflush(event->stream);
}

#pragma endregion

