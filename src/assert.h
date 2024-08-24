#ifndef clox_assert_h
#define clox_assert_h

#include <stdio.h>
#include <stdlib.h>

#define Assert(cond)                                                                               \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #cond, __FILE__,           \
                    __LINE__);                                                                     \
            exit(EXIT_FAILURE);                                                                    \
        }                                                                                          \
    } while (0)

#define Unreachable()                                                                              \
    do {                                                                                           \
        fprintf(stderr, "Unreachable code reached, file %s, line %d\n", __FILE__, __LINE__);       \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

#define Panic(message)                                                                             \
    do {                                                                                           \
        fprintf(stderr, "Panic: %s, file %s, line %d\n", message, __FILE__, __LINE__);             \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

#define Panicf(fmt, ...)                                                                           \
    do {                                                                                           \
        fprintf(stderr, "Panic: " fmt ", file %s, line %d\n", __VA_ARGS__, __FILE__, __LINE__);    \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

#endif