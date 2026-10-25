#include <string.h>

#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

static Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static char advance(void) {
    return *scanner.current++;
}

static char peek(void) {
    return *scanner.current;
}

static char peekNext(void) {
    if (peek() == '\0') {
        return '\0';
    }
    return scanner.current[1];
}

static void skipWhitespace(void) {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            default:
                return;
        }
    }
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

Token scanToken(void) {
    skipWhitespace();
    scanner.start = scanner.current;

    if (peek() == '\0') {
        return makeToken(TOKEN_EOF);
    }

    char c = advance();

    if (isDigit(c)) {
        while (isDigit(peek())) {
            advance();
        }

        if (peek() == '.' && isDigit(peekNext())) {
            advance();
            while (isDigit(peek())) {
                advance();
            }
        }

        return makeToken(TOKEN_NUMBER);
    }

    switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '+': return makeToken(TOKEN_PLUS);
        case '-': return makeToken(TOKEN_MINUS);
        case '*': return makeToken(TOKEN_STAR);
        case '/': return makeToken(TOKEN_SLASH);
        default: return errorToken("Unexpected character.");
    }
}
