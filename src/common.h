#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "program.h"

#define EXIT_COMPILE_ERROR 65
#define EXIT_RUNTIME_ERROR 70

#ifdef __GNUC__
#define FLEXIBLE_ARRAY_MEMBER
#else
#define FLEXIBLE_ARRAY_MEMBER 1
#endif

#endif