#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "array.h"
#include "assert.h"
#include "scanner.h"

#define SCANNER_ARENA_INITIAL_SIZE 1024
#define DEFAULT_KEYWORDS_CAPACITY 64

#define TOKEN(_type, _start, _length, _line)                                                       \
    (Token) {                                                                                      \
        .type = (_type), .start = (_start), .length = (_length), .line = (_line)                   \
    }

#pragma region Declare

static inline const char *peek(Scanner *scanner);
static inline const char *peekN(Scanner *scanner, size_t count);
static inline bool match(Scanner *scanner, const char ch);
static inline const char *advance(Scanner *scanner);
static inline bool eof(Scanner *scanner);
static KeywordTrieNode *build_keywords(Scanner *scanner);
static void insert_keyword(Vector *vec, KeywordTrieNode *root, const char *keyword, TokenType type);
static Token scan_error(Scanner *scanner, int line, const char *fmt, ...);
static Token scan_string(Scanner *scanner);
static Token scan_identifier(Scanner *scanner);
static Token scan_number(Scanner *scanner);
static Token scan_comment(Scanner *scanner);
static KeywordTrieNode *is_keyword(Scanner *scanner, const char *word, int length);
static inline bool is_digit(char ch);
static inline bool is_alpha(char ch);

#pragma endregion

#pragma region Public

void scanner_init(Scanner *scanner, Allocator *alloc, const char *source) {
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
    scanner->alloc = alloc;
    vector_init(&scanner->keywords_vec, alloc, DEFAULT_KEYWORDS_CAPACITY, sizeof(KeywordTrieNode));
    scanner->keywords = build_keywords(scanner);
}

void scanner_destroy(Scanner *scanner) {
    vector_destroy(&scanner->keywords_vec);
}

Token scanner_scan(Scanner *scanner) {
    const char *start = scanner->current;

    for (bool skip = true; skip;) {
        start = peek(scanner);
        if (eof(scanner))
            return TOKEN(TOKEN_EOF, NULL, 0, scanner->line);
        switch (*start) {
        case '\n':
            scanner->line++;
        case ' ':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
            advance(scanner);
            break;
        default:
            skip = false;
            break;
        }
    }

    switch (*start) {
    case '(':
        return TOKEN(TOKEN_LEFT_PAREN, advance(scanner), 1, scanner->line);
    case ')':
        return TOKEN(TOKEN_RIGHT_PAREN, advance(scanner), 1, scanner->line);
    case '{':
        return TOKEN(TOKEN_LEFT_BRACE, advance(scanner), 1, scanner->line);
    case '}':
        return TOKEN(TOKEN_RIGHT_BRACE, advance(scanner), 1, scanner->line);
    case ',':
        return TOKEN(TOKEN_COMMA, advance(scanner), 1, scanner->line);
    case '.':
        return TOKEN(TOKEN_DOT, advance(scanner), 1, scanner->line);
    case '+':
        return TOKEN(TOKEN_PLUS, advance(scanner), 1, scanner->line);
    case '-':
        return TOKEN(TOKEN_MINUS, advance(scanner), 1, scanner->line);
    case ';':
        return TOKEN(TOKEN_SEMICOLON, advance(scanner), 1, scanner->line);
    case '*':
        return TOKEN(TOKEN_STAR, advance(scanner), 1, scanner->line);
    case '!':
        advance(scanner);
        return match(scanner, '=') ? TOKEN(TOKEN_BANG_EQUAL, start, 2, scanner->line)
                                   : TOKEN(TOKEN_BANG, start, 1, scanner->line);
    case '=':
        advance(scanner);
        return match(scanner, '=') ? TOKEN(TOKEN_EQUAL_EQUAL, start, 2, scanner->line)
                                   : TOKEN(TOKEN_EQUAL, start, 1, scanner->line);
    case '>':
        advance(scanner);
        return match(scanner, '=') ? TOKEN(TOKEN_GREATER_EQUAL, start, 2, scanner->line)
                                   : TOKEN(TOKEN_GREATER, start, 1, scanner->line);
    case '<':
        advance(scanner);
        return match(scanner, '=') ? TOKEN(TOKEN_LESS_EQUAL, start, 2, scanner->line)
                                   : TOKEN(TOKEN_LESS, start, 1, scanner->line);
    case '/':
        advance(scanner);
        return match(scanner, '/') ? scan_comment(scanner)
                                   : TOKEN(TOKEN_SLASH, start, 1, scanner->line);
    case '"':
        advance(scanner);
        return scan_string(scanner);
    }

    if (is_alpha(*start))
        return scan_identifier(scanner);
    if (is_digit(*start))
        return scan_number(scanner);

    String *error = string_sprintf(scanner->alloc, "Unexpected character '%c'", *peek(scanner));
    Assert(error != NULL);
    return TOKEN(TOKEN_ERROR, error->data, 0, scanner->line);
}

String *token_repr(Token *token, Allocator *alloc) {
    return string_sprintf(alloc, "Token { type=%s, start=\"%.*s\", length=%d, line=%d }",
                          token_type_name(token->type), token->length, token->start, token->length,
                          token->line);
}

#pragma endregion

#pragma region Private

static inline const char *peek(Scanner *scanner) {
    return scanner->current;
}

static inline bool match(Scanner *scanner, const char ch) {
    if (eof(scanner))
        return false;
    if (*peek(scanner) != ch)
        return false;
    advance(scanner);
    return true;
}

static inline const char *peekN(Scanner *scanner, size_t count) {
    return scanner->current + count;
}

static inline const char *advance(Scanner *scanner) {
    return scanner->current++;
}

static inline bool eof(Scanner *scanner) {
    return *scanner->current == '\0';
}

static KeywordTrieNode *build_keywords(Scanner *scanner) {
    Vector *vec = &scanner->keywords_vec;
    // KeywordTrieNode* root = (KeywordTrieNode*)vector_append(vec, &(KeywordTrieNode) {0});
    KeywordTrieNode _root = { 0 };
    _root.ch = '?';
    KeywordTrieNode *root = (KeywordTrieNode *)vector_append(vec, &_root);
    insert_keyword(vec, root, "and", TOKEN_AND);
    insert_keyword(vec, root, "class", TOKEN_CLASS);
    insert_keyword(vec, root, "else", TOKEN_ELSE);
    insert_keyword(vec, root, "false", TOKEN_FALSE);
    insert_keyword(vec, root, "for", TOKEN_FOR);
    insert_keyword(vec, root, "fun", TOKEN_FUN);
    insert_keyword(vec, root, "if", TOKEN_IF);
    insert_keyword(vec, root, "nil", TOKEN_NIL);
    insert_keyword(vec, root, "or", TOKEN_OR);
    insert_keyword(vec, root, "print", TOKEN_PRINT);
    insert_keyword(vec, root, "return", TOKEN_RETURN);
    insert_keyword(vec, root, "super", TOKEN_SUPER);
    insert_keyword(vec, root, "this", TOKEN_THIS);
    insert_keyword(vec, root, "true", TOKEN_TRUE);
    insert_keyword(vec, root, "var", TOKEN_VAR);
    insert_keyword(vec, root, "while", TOKEN_WHILE);
    return root;
}

static void insert_keyword(Vector *vec, KeywordTrieNode *root, const char *keyword,
                           TokenType type) {
    // TODO: fix bug here
    KeywordTrieNode *iter = root;
    while (*keyword) {
        Assert(is_alpha(*keyword));
        int index = *keyword >= 'a' ? *keyword - 'a' : *keyword - 'A';
        KeywordTrieNode *next = iter->children[index];
        if (next == NULL) {
            next = (KeywordTrieNode *)vector_append(vec, &(KeywordTrieNode){ 0 });
            Assert(next != NULL);
            next->ch = *keyword;
            iter->children[index] = next;
        }
        iter = next;
        keyword++;
    }
    Assert(iter->end == false);
    iter->end = true;
    iter->type = type;
}

static Token scan_error(Scanner *scanner, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String *error = string_sprintf(scanner->alloc, fmt, args);
    va_end(args);
    Assert(error != NULL);
    return TOKEN(TOKEN_ERROR, error->data, 0, line);
}

static Token scan_string(Scanner *scanner) {
    int line = scanner->line;
    const char *start = advance(scanner); // consume start quote
    const char *ch = advance(scanner);
    for (; !eof(scanner) && *ch != '"'; ch = peek(scanner)) {
        if (*ch == '\n')
            scanner->line++;
        advance(scanner);
    }
    if (eof(scanner))
        return scan_error(scanner, line, "Unterminated string");
    advance(scanner); // consume end quote
    return TOKEN(TOKEN_STRING, start, ch - start, line);
}

static Token scan_identifier(Scanner *scanner) {
    const char *start = advance(scanner);
    Assert(is_alpha(*start));
    const char *ch = peek(scanner);
    for (; !eof(scanner) && (is_digit(*ch) || is_alpha(*ch)); ch = peek(scanner)) {
        advance(scanner);
    }
    KeywordTrieNode *node = is_keyword(scanner, start, ch - start);
    TokenType type = node != NULL ? node->type : TOKEN_IDENTIFIER;
    return TOKEN(type, start, ch - start, scanner->line);
}

static Token scan_number(Scanner *scanner) {
    int line = scanner->line;
    const char *start = advance(scanner);
    Assert(is_digit(*start));
    const char *ch = peek(scanner);
    for (; !eof(scanner) && is_digit(*ch); ch = peek(scanner))
        advance(scanner);
    if (*ch == '.' && is_digit(*peekN(scanner, 1))) {
        advance(scanner); // consume dot
        ch = advance(scanner);
        for (; !eof(scanner) && is_digit(*ch); ch = peek(scanner))
            advance(scanner);
    }
    return TOKEN(TOKEN_NUMBER, start, ch - start, line);
}

static Token scan_comment(Scanner *scanner) {
    int line = scanner->line;
    const char *start = advance(scanner);
    const char *ch = peek(scanner);
    for (; !eof(scanner) && *ch != '\n'; ch = peek(scanner)) {
        advance(scanner);
    }
    return TOKEN(TOKEN_COMMENT, start, ch - start, line);
}

static KeywordTrieNode *is_keyword(Scanner *scanner, const char *word, int length) {
    KeywordTrieNode *iter = scanner->keywords;
    for (int i = 0; iter != NULL && i < length; i++) {
        char ch = word[i];
        if (ch == '\0')
            return NULL;
        Assert(is_alpha(ch));
        int index = ch >= 'a' ? ch - 'a' : ch - 'A';
        iter = iter->children[index];
    }
    return iter && iter->end ? iter : NULL;
}

static inline bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

static inline bool is_alpha(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

#pragma endregion
