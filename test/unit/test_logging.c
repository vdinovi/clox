#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "unity.h"
#include "helpers.h"
#include "allocator.h"
#include "logging.h"
#include "assert.h"

Allocator alloc;
Logger alloc_logger;
Logger logger;

static struct {
    FILE *file;
    size_t size;
    char *buffer;
    char *line;
} outstream = {0};

static String* next_log() {
    char *start = outstream.line;
    char *end;
    for (end = start; *end != '\n'; end++) {}
    String *string = string_alloc(&alloc, end - start + 1);
    for (char *ch = start; ch != end; ch++) {
        string->data[string->length++] = *ch;
    }
    outstream.line = end;
    return string;
}

void setUp(void) {
    allocator_init(&alloc, &alloc_logger);
    logger_init(&alloc_logger, stderr, -1, &alloc);

    outstream.file = open_memstream(&outstream.buffer, &outstream.size);
    TEST_ASSERT_NOT_NULL(outstream.file);
    TEST_ASSERT_NOT_NULL(outstream.buffer);
    logger_init(&logger, outstream.file, LOG_LEVEL_INFO, &alloc);
    outstream.line = outstream.buffer;
}

void tearDown(void) {
    logger_destroy(&alloc_logger);
    allocator_destroy(&alloc);

    logger_destroy(&logger);
    fclose(outstream.file);
    free(outstream.buffer);
}

#define TEST_ASSERT_EQUAL_IS_DIGIT(ch) TEST_ASSERT_TRUE_MESSAGE(isdigit((ch)), "expected digit")

static void assert_log_string(const char *log, const char *message, LogLevel level, const char *filename, int line) {
    const char *ch = log;
    // assert timestamp
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
    while (*ch == ' ') ch++;

    // assert log level
    const char *level_name = log_level_name(level);
    size_t level_name_length = strlen(level_name);
    TEST_ASSERT_EQUAL_STRING_LEN(level_name, ch, level_name_length);
    ch += level_name_length;
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(' ', *ch++, "expected ' '");
    while (*ch == ' ') ch++;

    // assert file name
    size_t filename_len = strlen(filename);
    TEST_ASSERT_EQUAL_STRING_LEN(filename, ch, filename_len);
    ch += filename_len;
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(':', *ch++, "expected ':'");

    // assert line number
    static char line_str[16];
    size_t line_str_length = snprintf(line_str, sizeof(line_str), "%d", line);
    TEST_ASSERT_EQUAL_STRING_LEN(line_str, ch, line_str_length);
    ch += level_name_length;
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(':', *ch++, "expected ':'");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(' ', *ch++, "expected ' '");

    // assert message
    size_t message_len = strlen(message);
    TEST_ASSERT_EQUAL_STRING_LEN(message, ch, message_len);
    ch += message_len;
    TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', *ch++, "expected '\0'");
}

#define FILE "foo.c"
#define LINE 1234
#define FMT "int=%d double=%lf size_t=%zu char=%c ptr=%p str=%s"
#define ARGS 32, 3.14, (size_t)256, '?', (void*)&alloc, "test string"

void test_logger(void) {
    static char message[4096];
    
    for (LogLevel level = LOG_LEVEL_MIN; level <= LOG_LEVEL_MAX; level++) {
        logger_emit(&logger, level, FILE, LINE, FMT, ARGS);
        outstream.line = outstream.buffer;
        String *log = next_log();
        TEST_ASSERT_NOT_NULL(log->data);

        snprintf(message, sizeof(message), FMT, ARGS);
        assert_log_string(log->data, message, level, FILE, LINE);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_logger);
    return UNITY_END();
}