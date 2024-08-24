#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "assert.h"
#include "helpers.h"
#include "logging.h"
#include "unity.h"

#define TEST_ASSERT_EQUAL_IS_DIGIT(ch) TEST_ASSERT_TRUE_MESSAGE(isdigit((ch)), "expected digit")

static struct {
    FILE *file;
    size_t size;
    char *buffer;
    char *line;
} outstream = { 0 };

static T t;

static Logger logger;

static String *next_log();
static inline const char *assert_log_timestamp(const char *ch);
static inline const char *assert_log_level(const char *ch, LogLevel level);
static inline const char *assert_log_location(const char *ch, const char *filename, int line);
static char *assert_log_matches(const char *log, const char *message, LogLevel level,
                                const char *filename, int line);

void setUp(void) {
    setup(&t);

    outstream.file = open_memstream(&outstream.buffer, &outstream.size);
    TEST_ASSERT_NOT_NULL(outstream.file);
    TEST_ASSERT_NOT_NULL(outstream.buffer);
    logger_init(&logger, outstream.file, LOG_LEVEL_MIN);
    outstream.line = outstream.buffer;
}

void tearDown(void) {
    logger_destroy(&logger);
    fclose(outstream.file);
    free(outstream.buffer);

    teardown(&t);
}

#define FILE "foo.c"
#define LINE 1234
#define FMT "int=%d double=%lf size_t=%zu char=%c ptr=%p str=%s"
#define ARGS 32, 3.14, (size_t)256, '?', (void *)&t.alloc, "test string"

void test_logger(void) {
    static char message[4096];

    for (LogLevel level = LOG_LEVEL_MIN; level <= LOG_LEVEL_MAX; level++) {
        logger_emit(&logger, level, FILE, LINE, FMT, ARGS);
    }
    outstream.line = outstream.buffer;
    for (LogLevel level = LOG_LEVEL_MIN; level <= LOG_LEVEL_MAX; level++) {
        String *log = next_log();
        TEST_ASSERT_NOT_NULL(log->data);

        snprintf(message, sizeof(message), FMT, ARGS);
        outstream.line = assert_log_matches(log->data, message, level, FILE, LINE);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_logger);
    return UNITY_END();
}

static String *next_log() {
    char *start = outstream.line;
    char *end = start;
    while (*end != '\n') {
        end++;
    }
    String *string = string_alloc(&t.alloc, end - start + 1);
    for (char *ch = start; ch != end; ch++) {
        string->data[string->length++] = *ch;
    }
    outstream.line = end;
    return string;
}

static inline const char *assert_log_timestamp(const char *ch) {
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(':', *ch++, "expected ':'");
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(':', *ch++, "expected ':'");
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_CHAR_MESSAGE('.', *ch++, "expected '.'");
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_IS_DIGIT(*ch++);
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(' ', *ch++, "expected ' '");
    return ch;
}

static inline const char *assert_log_level(const char *ch, LogLevel level) {
    const char *level_name = LOG_LEVEL_NAMES[level];
    size_t level_name_length = strlen(level_name);
    TEST_ASSERT_EQUAL_STRING_LEN(level_name, ch, level_name_length);
    ch += level_name_length;
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(' ', *ch++, "expected ' '");
    return ch;
}

static inline const char *assert_log_location(const char *ch, const char *filename, int line) {
    size_t filename_len = strlen(filename);
    TEST_ASSERT_EQUAL_STRING_LEN(filename, ch, filename_len);
    ch += filename_len;
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(':', *ch++, "expected ':'");

    static char line_str[16];
    size_t line_str_length = snprintf(line_str, sizeof(line_str), "%d", line);
    TEST_ASSERT_EQUAL_STRING_LEN(line_str, ch, line_str_length);
    ch += line_str_length;
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(':', *ch++, "expected ':'");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(' ', *ch++, "expected ' '");
    return ch;
}

static char *assert_log_matches(const char *log, const char *message, LogLevel level,
                                const char *filename, int line) {
    const char *ch = log;

    ch = assert_log_timestamp(ch);
    while (*ch == ' ')
        ch++;

    ch = assert_log_level(ch, level);
    while (*ch == ' ')
        ch++;

    ch = assert_log_location(ch, filename, line);

    size_t message_len = strlen(message);
    TEST_ASSERT_EQUAL_STRING_LEN(message, ch, message_len);
    ch += message_len;
    return (char *)ch;
}