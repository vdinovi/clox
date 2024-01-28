#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "logging.h"
#include "opcode.h"
#include "allocator.h"
#include "vm.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define PATCH_VERSION 0

// TODO: figure out whats going on with this
// #define STRINGIFY(x) #x
// #define SEMANTIC_VERSION STRINGIFY(MAJOR_VERSION) "."  STRINGIFY(MINOR_VERSION) "." STRINGIFY(PATCH_VERSION)
#define SEMANTIC_VERSION "0.1.0"

#define MAX_INPUT_FILE_BYTES 1024 * 1024

#pragma region Declare

static struct {
    const char *program;
    const char *input;
    enum { REPL, EXEC } mode;
    bool debug;
    bool trace;
} config = {
    .program = NULL,
    .input = NULL,
    .mode = REPL,
    .debug = false,
    .trace = false,
};

static void usage(FILE *out, const char *program);
static void parse(int argc, char *argv[]);
static void setup();
static void start_repl();
static void exec_file();
static const char* read_file(Allocator *alloc, FILE *file, size_t max_bytes);

#pragma endregion

#pragma region Public

int main(int argc, char *argv[]) {
    parse(argc, argv);
    setup();

    if (config.mode == REPL) {
        start_repl();
    } else {
        exec_file();
    }
    return EXIT_SUCCESS;
}

#pragma endregion

#pragma region Private

static void usage(FILE *out, const char *program) {
    fprintf(out, "Usage: %s [OPTIONS]... <input_file>\n", program);
    fprintf(out, "Interpreter for the lox programming language\n\n");
    fprintf(out, "Options:\n");
    fprintf(out, "  -h, --help      Display this help message and exit\n");
    fprintf(out, "  -v, --version   Output version information and exit\n");
    fprintf(out, "  --debug         Emit verbose debug information to stderr\n");
    fprintf(out, "  --trace         Emit very verbose debug information to stderr\n");
    fprintf(out, "  <input_file>    The input file (positional argument)\n");
    fprintf(out, "");
    fprintf(out, "\nExamples\n");
    fprintf(out, "");
    fprintf(out, "  %s program.lox (executes the content of program.lox)", program);
    fprintf(out, "  %s             (begins a REPL interpreter)", program);
}

static void parse(int argc, char *argv[]) {
    int optind;

    for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
        case 'h':
            usage(stdout, argv[0]);
            exit(EXIT_SUCCESS);
        case 'v':
            fprintf(stdout, "clox %s\n", SEMANTIC_VERSION);
            exit(EXIT_SUCCESS);
        case '-':
            if (strcmp(argv[optind], "--help") == 0) {
                usage(stdout, argv[0]);
                exit(EXIT_SUCCESS);
            } else if (strcmp(argv[optind], "--version") == 0) {
                printf("clox %s\n", SEMANTIC_VERSION);
                exit(EXIT_SUCCESS);
            } else if (strcmp(argv[optind], "--debug") == 0) {
                config.debug = true;
            } else if (strcmp(argv[optind], "--trace") == 0) {
                config.trace = true;
            } else {
                usage(stderr, argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            usage(stderr, argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    size_t num_args = argc - optind;
    config.program = argv[0];

    switch (num_args) {
    case 0:
        config.mode = REPL;
        config.input = NULL;
        break;
    case 1:
        config.mode = EXEC;
        config.input = argv[optind];
        break;
    default:
        usage(stderr, argv[0]);
        exit(EXIT_FAILURE);
    }
}

static void setup() {
    if (config.trace) {
        set_log_level(LOG_TRACE);
    } else if (config.debug) {
        set_log_level(LOG_DEBUG);
    } else {
        set_log_level(LOG_INFO);
    }
}

static void start_repl() {
    int exit_code = EXIT_SUCCESS;
    Allocator alloc = {0};
    allocator_init(&alloc);
    VirtualMachine vm;
    virtual_machine_init(&vm, &alloc);
    log_debug("starting (vm=%p)", &vm);

    char line[1024] = {0};
    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        line[strcspn(line, "\n")] = '\0';

        InterpretResult result = interpret(&vm, line);
        if (result == INTERPRET_COMPILE_ERROR) {
            log_debug("Compile Error");
            exit_code = EXIT_COMPILE_ERROR;
            break;
        }
    }
    allocator_destroy(&alloc);
    exit(exit_code);
}

static void exec_file() {
    int exit_code = EXIT_SUCCESS;
    Allocator alloc = {0};
    allocator_init(&alloc);
    VirtualMachine vm = {0};
    virtual_machine_init(&vm, &alloc);
    log_debug("starting (vm=%p)", &vm);

    Assert(config.input != NULL);
    FILE *file = fopen(config.input, "r");
    if (file == NULL) {
        perror("failed to open input file");
        exit(EXIT_FAILURE);
    }
    const char* contents = read_file(&alloc, file, MAX_INPUT_FILE_BYTES);

    InterpretResult result = interpret(&vm, contents);
    if (result == INTERPRET_COMPILE_ERROR) {
        log_debug("Compile Error");
        exit_code = EXIT_COMPILE_ERROR;
        goto cleanup;
    }

    if (result == INTERPRET_RUNTIME_ERROR) {
        log_debug("Runtime Error");
        exit(EXIT_RUNTIME_ERROR);

        exit_code = EXIT_COMPILE_ERROR;
        goto cleanup;
    }

cleanup:
    allocator_destroy(&alloc);
    if (fclose(file) != 0) {
        perror("failed to close input file");
        exit(EXIT_FAILURE);
    }
    exit(exit_code);
}

static const char* read_file(Allocator *alloc, FILE *file, size_t max_bytes) {
    size_t cap = 1024;
    char* buffer = (char*)allocator_alloc(alloc, cap);
    char *current = buffer;

    for (;;) {
        size_t len = current - buffer;
        size_t available = cap - len;
        size_t bytes_read = fread(current, 1, available, file);
        current += bytes_read;
        len += bytes_read;
        available = cap - len;
        if (bytes_read < len) {
            if (feof(file)) {
                if (available == 0) {
                    buffer = (char*)allocator_realloc(alloc, buffer, cap, cap + 1);
                }
                buffer[len] = '\0';
                break;
            }
            if (ferror(file)) {
                perror("failed to read input file");
                exit(EXIT_FAILURE);
            }
        }
        size_t new_cap = cap * 2;
        if (new_cap > max_bytes) {
            fprintf(stderr, "Error: input file exceeds maximum size of %zu bytes\n", max_bytes);
            exit(EXIT_FAILURE);
        }
        buffer = (char*)allocator_realloc(alloc, buffer, cap, new_cap);
        cap = new_cap;
    }
    return buffer;
}

#pragma endregion
