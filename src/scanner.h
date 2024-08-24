#ifndef clox_scanner_h
#define clox_scanner_h

#include <stdbool.h>

#include "array.h"
#include "assert.h"
#include "vector.h"

#define ALPHABET_SIZE 26
typedef enum TokenType {
    TOKEN_BYTE,
    // 1 character tokens
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,
    // 1 or 2 character tokens
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    // Literals
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    // Keywords
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    TOKEN_COMMENT,

    TOKEN_ERROR,
    TOKEN_EOF,
} TokenType;

typedef struct Token {
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

typedef struct KeywordTrieNode {
    char ch;
    TokenType type;
    bool end;
    int num_children;
    struct KeywordTrieNode *children[ALPHABET_SIZE];
} KeywordTrieNode;

typedef struct Scanner {
    const char *start;
    const char *current;
    int line;
    KeywordTrieNode *keywords;
    Vector keywords_vec;
    Allocator *alloc;
} Scanner;

void scanner_init(Scanner *scan, Allocator *alloc, const char *source);
void scanner_destroy(Scanner *scanner);
Token scanner_scan(Scanner *scan);

String *token_repr(Token *token, Allocator *alloc);

static inline const char *token_type_name(TokenType type) {
    switch (type) {
    case TOKEN_BYTE:
        return "BYTE";
    case TOKEN_LEFT_PAREN:
        return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TOKEN_LEFT_BRACE:
        return "LEFT_BRACE";
    case TOKEN_RIGHT_BRACE:
        return "RIGHT_BRACE";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_DOT:
        return "DOT";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_SLASH:
        return "SLASH";
    case TOKEN_STAR:
        return "STAR";
    case TOKEN_BANG:
        return "BANG";
    case TOKEN_BANG_EQUAL:
        return "BANG_EQUAL";
    case TOKEN_EQUAL:
        return "EQUAL";
    case TOKEN_EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_LESS_EQUAL:
        return "LESS_EQUAL";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_AND:
        return "AND";
    case TOKEN_CLASS:
        return "CLASS";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_FALSE:
        return "FALSE";
    case TOKEN_FOR:
        return "FOR";
    case TOKEN_FUN:
        return "FUN";
    case TOKEN_IF:
        return "IF";
    case TOKEN_NIL:
        return "NIL";
    case TOKEN_OR:
        return "OR";
    case TOKEN_PRINT:
        return "PRINT";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_SUPER:
        return "SUPER";
    case TOKEN_THIS:
        return "THIS";
    case TOKEN_TRUE:
        return "TRUE";
    case TOKEN_VAR:
        return "VAR";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_COMMENT:
        return "COMMENT";
    case TOKEN_ERROR:
        return "ERROR";
    case TOKEN_EOF:
        return "EOF";
    default:
        Panicf("Unknown token type %d", type);
    }
}

#endif